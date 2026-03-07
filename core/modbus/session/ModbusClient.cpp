#include "ModbusClient.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
#include <QCoreApplication>
#include <QEventLoop>

namespace modbus::session {

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
}

void ModbusClient::abort() {
    spdlog::info("ModbusClient: Abort requested");
    aborted_ = true;
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

    for (int i = 0; i <= retries; ++i) {
        if (aborted_) return ModbusResponse::Error("Aborted");

        lastResponse = sendRequestInternal(request, slaveId);
        if (lastResponse.isSuccess) {
            return lastResponse;
        }
        
        if (i < retries && !aborted_) {
            spdlog::warn("Request failed, retrying... ({}/{}) Error: {}", 
                         i + 1, retries, lastResponse.error.toStdString());
            std::this_thread::sleep_for(std::chrono::milliseconds(config_.retryIntervalMs));
        }
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
        return ModbusResponse::Error("Not connected");
    }

    // 1. 清理旧缓冲区和状态
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (aborted_) return ModbusResponse::Error("Aborted");
        buffer_.clear();
        responseReady_ = false;
        lastError_.clear();
    }

    // 2. 构建 ADU
    int targetSlaveId = (slaveId == -1) ? config_.slaveId : slaveId;
    QByteArray adu = transport_->buildRequest(request, targetSlaveId);

    // 3. 发送数据
    if (!channel_->write(adu)) {
        return ModbusResponse::Error("Write failed");
    }
    auto start = std::chrono::steady_clock::now();

    // 4. 等待响应
    // 使用绝对时间作为截止日期，避免循环中相对时间重置
    auto deadline = start + std::chrono::milliseconds(config_.timeoutMs);

    spdlog::info("ModbusClient: Entering wait loop, deadline in {}ms", config_.timeoutMs);
    
    while (true) {
        if (aborted_) {
            spdlog::info("ModbusClient: Aborted during wait");
            return ModbusResponse::Error("Aborted");
        }

        // 关键修复：允许在等待期间处理当前线程的信号。
        QCoreApplication::processEvents(QEventLoop::AllEvents);

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (responseReady_ || !lastError_.isEmpty()) {
                // 有信号或错误，进入后续处理
            } else if (std::chrono::steady_clock::now() >= deadline) {
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
                    return ModbusResponse::Error(QString("Modbus Exception: %1").arg(static_cast<int>(pdu->exceptionCode())));
                }
                if (pdu->originalFunctionCode() != request.functionCode()) {
                     continue; // 匹配失败，可能还有后续包，继续等待
                }
                
                auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
                return ModbusResponse::Success(*pdu, static_cast<int>(rttMs));
            } else {
                return ModbusResponse::Error("Response parsing failed");
            }
        } else if (integrity == -1) {
            buffer_.clear();
            return ModbusResponse::Error("Invalid data received");
        }
        
        // integrity == 0，数据尚不完整，继续循环等待直到 deadline
        if (std::chrono::steady_clock::now() >= deadline) {
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
