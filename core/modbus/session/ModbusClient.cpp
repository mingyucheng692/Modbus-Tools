#include "ModbusClient.h"
#include <spdlog/spdlog.h>
#include <chrono>

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

std::future<ModbusResponse> ModbusClient::sendRequest(const base::Pdu& request, int slaveId) {
    // 使用 async 启动一个任务来执行阻塞式请求，不阻塞调用者
    return std::async(std::launch::async, [this, request, slaveId]() {
        std::lock_guard<std::mutex> lock(requestMutex_); // 确保一次只处理一个请求
        
        // 重试逻辑
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
                // 可以加一点延时
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        return lastResponse;
    });
}

void ModbusClient::sendRaw(const QByteArray& data) {
    // 异步发送，避免阻塞调用线程
    std::thread([this, data]() {
        std::lock_guard<std::mutex> lock(requestMutex_); // 确保不与 sendRequest 冲突
        if (isConnected()) {
            channel_->write(data);
        }
    }).detach();
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
    if (!channel_->write(adu)) {
        return ModbusResponse::Error("Write failed");
    }

    // 4. 等待响应
    std::unique_lock<std::mutex> lock(mutex_);
    
    // 循环等待直到收到完整包或超时
    auto start = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(config_.timeoutMs);

    while (true) {
        if (cv_.wait_for(lock, timeout) == std::cv_status::timeout) {
            return ModbusResponse::Error("Timeout");
        }

        // 检查是否有错误
        if (!lastError_.isEmpty()) {
            return ModbusResponse::Error(lastError_);
        }

        // 检查完整性
        int integrity = transport_->checkIntegrity(buffer_);
        if (integrity > 0) {
            // 收到完整包
            auto pdu = transport_->parseResponse(buffer_);
            if (pdu) {
                // 检查是否是异常响应
                if (pdu->isException()) {
                    return ModbusResponse::Error(QString("Modbus Exception: %1")
                                                 .arg(static_cast<int>(pdu->exceptionCode())));
                }
                // 简单的请求/响应匹配：检查功能码
                // 注意：如果从站返回异常，功能码最高位被置1
                if (pdu->originalFunctionCode() != request.functionCode()) {
                     // 可能是迟到的包，或者是错误的包，继续等待？
                     // 简单起见，这里认为是协议错误
                     return ModbusResponse::Error(QString("Function code mismatch: sent %1, received %2")
                                                  .arg(static_cast<int>(request.functionCode()))
                                                  .arg(static_cast<int>(pdu->functionCode())));
                }
                return ModbusResponse::Success(*pdu);
            } else {
                return ModbusResponse::Error("Response parsing failed (CRC error or invalid format)");
            }
        } else if (integrity == -1) {
            // 数据错误，丢弃缓冲区重新开始？或者直接报错
            // 简单起见，清空缓冲区继续等待后续数据（如果是 TCP流），或者报错
            // 这里选择报错重试
            return ModbusResponse::Error("Invalid data received");
        }
        
        // integrity == 0，数据不完整，继续 wait
        
        // 更新剩余超时时间
        auto now = std::chrono::steady_clock::now();
        if (now - start > timeout) {
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
