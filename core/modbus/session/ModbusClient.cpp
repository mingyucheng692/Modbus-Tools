#include "ModbusClient.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
#include <QCoreApplication>
#include <QEventLoop>

namespace modbus::session {

const char* ModbusClient::toString(RequestState state) {
    switch (state) {
    case RequestState::Idle: return "Idle";
    case RequestState::Sending: return "Sending";
    case RequestState::Waiting: return "Waiting";
    case RequestState::Completed: return "Completed";
    case RequestState::Failed: return "Failed";
    case RequestState::Aborted: return "Aborted";
    }
    return "Unknown";
}

void ModbusClient::transitionTo(RequestState newState, const char* reason) {
    RequestState oldState = requestState_.exchange(newState);
    if (oldState == newState) {
        return;
    }
    spdlog::info("ModbusClient: state {} -> {} ({})",
                 toString(oldState),
                 toString(newState),
                 reason);
}

ModbusClient::RequestState ModbusClient::requestState() const {
    return requestState_.load();
}

ModbusClient::ModbusClient(std::shared_ptr<io::IChannel> channel, 
                           std::shared_ptr<transport::ITransport> transport)
    : channel_(std::move(channel)), transport_(std::move(transport)) {
    
    // 设置 Channel 回调
    channel_->setReadHandler([this](QByteArrayView data) {
        onDataReceived(data);
    });
    
    channel_->setErrorHandler([this](const QString& error) {
        onError(error);
    });
}

ModbusClient::~ModbusClient() {
    disconnect();
}

bool ModbusClient::connect() {
    if (!channel_) return false;
    
    // 应用超时配置
    io::Timeouts timeouts;
    timeouts.readMs = config_.timeoutMs;
    timeouts.writeMs = config_.timeoutMs;
    channel_->setTimeouts(timeouts);
    
    return channel_->open();
}

void ModbusClient::disconnect() {
    if (channel_) {
        channel_->close();
    }
    transitionTo(RequestState::Idle, "disconnect");
}

void ModbusClient::abort() {
    spdlog::info("ModbusClient: Abort requested");
    aborted_ = true;
    transitionTo(RequestState::Aborted, "abort");
    cv_.notify_all();
}

bool ModbusClient::isConnected() const {
    return channel_ && channel_->isOpen();
}

void ModbusClient::setConfig(const base::ModbusConfig& config) {
    config_ = config;
    if (isConnected()) {
        // 如果已经连接，动态更新超时
        io::Timeouts timeouts;
        timeouts.readMs = config_.timeoutMs;
        timeouts.writeMs = config_.timeoutMs;
        channel_->setTimeouts(timeouts);
    }
}

ModbusResponse ModbusClient::sendRequest(const base::Pdu& request, int slaveId) {
    std::lock_guard<std::recursive_mutex> lock(requestMutex_);
    
    int retries = config_.retries;
    ModbusResponse lastResponse = ModbusResponse::Error("Unknown error");
    transitionTo(RequestState::Idle, "request-start");

    for (int i = 0; i <= retries; ++i) {
        if (aborted_) {
            transitionTo(RequestState::Aborted, "request-aborted-before-send");
            return ModbusResponse::Error("Aborted");
        }

        lastResponse = sendRequestInternal(request, slaveId);
        if (lastResponse.isSuccess) {
            transitionTo(RequestState::Completed, "request-success");
            return lastResponse;
        }
        
        if (i < retries && !aborted_) {
            transitionTo(RequestState::Failed, "request-retry");
            spdlog::warn("Request failed, retrying... ({}/{}) Error: {}", 
                         i + 1, retries, lastResponse.error.toStdString());
            std::this_thread::sleep_for(std::chrono::milliseconds(config_.retryIntervalMs));
        }
    }
    if (aborted_) {
        transitionTo(RequestState::Aborted, "request-aborted-after-retries");
    } else {
        transitionTo(RequestState::Failed, "request-failed");
    }
    return lastResponse;
}

void ModbusClient::sendRaw(const QByteArray& data) {
    std::lock_guard<std::recursive_mutex> lock(requestMutex_);
    if (isConnected()) {
        channel_->write(data);
    }
}

ModbusResponse ModbusClient::sendRequestInternal(const base::Pdu& request, int slaveId) {
    if (!isConnected()) {
        transitionTo(RequestState::Failed, "not-connected");
        return ModbusResponse::Error("Not connected");
    }

    // 1. 清理旧缓冲区和状态
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (aborted_) {
            transitionTo(RequestState::Aborted, "aborted-before-build");
            return ModbusResponse::Error("Aborted");
        }
        buffer_.clear();
        responseReady_ = false;
        lastError_.clear();
    }

    // 2. 构建 ADU
    int targetSlaveId = (slaveId == -1) ? config_.slaveId : slaveId;
    QByteArray adu = transport_->buildRequest(request, targetSlaveId);

    // 3. 发送数据
    if (!channel_->write(adu)) {
        transitionTo(RequestState::Failed, "write-failed");
        return ModbusResponse::Error("Write failed");
    }
    transitionTo(RequestState::Sending, "write-success");
    auto start = std::chrono::steady_clock::now();

    // 4. 等待响应
    // 使用绝对时间作为截止日期，避免循环中相对时间重置
    auto deadline = start + std::chrono::milliseconds(config_.timeoutMs);
    transitionTo(RequestState::Waiting, "wait-response");

    spdlog::info("ModbusClient: Entering wait loop, deadline in {}ms", config_.timeoutMs);
    
    while (true) {
        if (aborted_) {
            spdlog::info("ModbusClient: Aborted during wait");
            transitionTo(RequestState::Aborted, "aborted-during-wait");
            return ModbusResponse::Error("Aborted");
        }

        // 关键修复：允许在等待期间处理当前线程的信号。
        QCoreApplication::processEvents(QEventLoop::AllEvents);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (responseReady_ || !lastError_.isEmpty()) {
                // 有信号或错误，进入后续处理
            } else if (std::chrono::steady_clock::now() >= deadline) {
                transitionTo(RequestState::Failed, "timeout");
                return ModbusResponse::Error("Timeout");
            } else {
                // 继续等待信号
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
        }

        std::unique_lock<std::mutex> lock(mutex_);
        // 检查是否有错误
        if (!lastError_.isEmpty()) {
            QString err = lastError_;
            lastError_.clear();
            transitionTo(RequestState::Failed, "channel-error");
            return ModbusResponse::Error(err);
        }

        // 重置准备就绪标志，开始检查数据
        responseReady_ = false;

        int integrity = transport_->checkIntegrity(buffer_);
        if (integrity > 0) {
            QByteArray frame = buffer_.left(integrity);
            buffer_.remove(0, integrity); 
            
            auto pdu = transport_->parseResponse(frame);
            if (pdu) {
                if (pdu->isException()) {
                    transitionTo(RequestState::Failed, "modbus-exception");
                    return ModbusResponse::Error(QString("Modbus Exception: %1").arg(static_cast<int>(pdu->exceptionCode())));
                }
                if (pdu->originalFunctionCode() != request.functionCode()) {
                     continue; // 匹配失败，可能还有后续包，继续等待
                }
                
                auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
                transitionTo(RequestState::Completed, "response-parsed");
                return ModbusResponse::Success(*pdu, static_cast<int>(rttMs));
            } else {
                transitionTo(RequestState::Failed, "response-parse-failed");
                return ModbusResponse::Error("Response parsing failed");
            }
        } else if (integrity == -1) {
            buffer_.clear();
            transitionTo(RequestState::Failed, "integrity-failed");
            return ModbusResponse::Error("Invalid data received");
        }
        
        // integrity == 0，数据尚不完整，继续循环等待直到 deadline
        if (std::chrono::steady_clock::now() >= deadline) {
             transitionTo(RequestState::Failed, "timeout-full-packet");
             return ModbusResponse::Error("Timeout while waiting for full packet");
        }
    }
}

void ModbusClient::onDataReceived(QByteArrayView data) {
    std::lock_guard<std::mutex> lock(mutex_);
    spdlog::info("ModbusClient: Data received, size={}, notifying loop", data.size());
    buffer_.append(data);
    responseReady_ = true; // 即使不完整也唤醒，由工作线程检查完整性
    cv_.notify_one();
}

void ModbusClient::onError(const QString& error) {
    std::lock_guard<std::mutex> lock(mutex_);
    lastError_ = error;
    cv_.notify_one();
}

} // namespace modbus::session
