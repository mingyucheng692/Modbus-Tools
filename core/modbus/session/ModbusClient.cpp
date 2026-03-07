#include "ModbusClient.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>

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
    std::lock_guard<std::mutex> lock(requestMutex_);
    
    int retries = config_.retries;
    ModbusResponse lastResponse = ModbusResponse::Error("Unknown error");

    for (int i = 0; i <= retries; ++i) {
        lastResponse = sendRequestInternal(request, slaveId);
        if (lastResponse.isSuccess) {
            return lastResponse;
        }
        
        if (i < retries) {
            spdlog::warn("Request failed, retrying... ({}/{}) Error: {}", 
                         i + 1, retries, lastResponse.error.toStdString());
            std::this_thread::sleep_for(std::chrono::milliseconds(config_.retryIntervalMs));
        }
    }
    return lastResponse;
}

void ModbusClient::sendRaw(const QByteArray& data) {
    std::lock_guard<std::mutex> lock(requestMutex_);
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
        buffer_.clear();
        responseReady_ = false;
        lastError_.clear();
    }

    // 2. 构建 ADU
    int targetSlaveId = (slaveId == -1) ? config_.slaveId : slaveId;
    QByteArray adu = transport_->buildRequest(request, targetSlaveId);

    // 3. 发送数据
    auto start = std::chrono::steady_clock::now();
    if (!channel_->write(adu)) {
        return ModbusResponse::Error("Write failed");
    }

    // 4. 等待响应
    std::unique_lock<std::mutex> lock(mutex_);
    
    // 使用绝对时间作为截止日期，避免循环中相对时间重置
    auto deadline = start + std::chrono::milliseconds(config_.timeoutMs);

    while (true) {
        // 使用带谓词的 wait_until，原子化检查条件，防止虚假唤醒和竞态
        bool signaled = cv_.wait_until(lock, deadline, [this]() {
            return responseReady_ || !lastError_.isEmpty();
        });

        if (!signaled) {
            return ModbusResponse::Error("Timeout");
        }

        // 检查是否有错误
        if (!lastError_.isEmpty()) {
            QString err = lastError_;
            lastError_.clear(); // 清理状态槽为下次做准备
            return ModbusResponse::Error(err);
        }

        // 重置准备就绪标志
        responseReady_ = false;

        // 检查数据完整性
        int integrity = transport_->checkIntegrity(buffer_);
        if (integrity > 0) {
            // 收到完整包
            QByteArray frame = buffer_.left(integrity);
            // 关键改进：仅从缓冲区移除已处理数据，保留后续可能存在的粘包数据
            buffer_.remove(0, integrity); 
            
            auto pdu = transport_->parseResponse(frame);
            if (pdu) {
                // 检查是否是异常响应
                if (pdu->isException()) {
                    return ModbusResponse::Error(QString("Modbus Exception: %1")
                                                 .arg(static_cast<int>(pdu->exceptionCode())));
                }
                // 请求/响应匹配检查
                if (pdu->originalFunctionCode() != request.functionCode()) {
                     // 如果功能码不匹配，可能是迟到的干扰包，继续在截止时间内等待
                     continue;
                }
                
                // 计算 RTT (基于底层发送完成到接收完整解析的时间)
                auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start).count();
                return ModbusResponse::Success(*pdu, static_cast<int>(rttMs));
            } else {
                return ModbusResponse::Error("Response parsing failed (Invalid format)");
            }
        } else if (integrity == -1) {
            // 协议解析发生严重错误，需清空缓冲区
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
