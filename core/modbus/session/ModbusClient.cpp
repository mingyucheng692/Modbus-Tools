#include "ModbusClient.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
#include <algorithm>

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

int ModbusClient::enqueuePendingRequest(const base::Pdu& request, int slaveId) {
    std::lock_guard<std::mutex> pendingLock(pendingMutex_);
    PendingRequest item;
    item.requestId = nextRequestId_++;
    item.functionCode = request.functionCode();
    item.slaveId = (slaveId == -1) ? config_.slaveId : slaveId;
    item.timeoutMs = config_.timeoutMs;
    item.retries = config_.retries;
    item.enqueueAt = std::chrono::steady_clock::now();
    pendingRequests_.push_back(item);
    spdlog::info("ModbusClient: enqueue request id={}, fc={}, slave={}, queue={}",
                 item.requestId,
                 static_cast<int>(item.functionCode),
                 item.slaveId,
                 pendingRequests_.size());
    return item.requestId;
}

void ModbusClient::finishPendingRequest(int requestId, bool success, const QString& error) {
    std::lock_guard<std::mutex> pendingLock(pendingMutex_);
    auto it = std::find_if(pendingRequests_.begin(), pendingRequests_.end(), [requestId](const PendingRequest& item) {
        return item.requestId == requestId;
    });
    if (it == pendingRequests_.end()) {
        spdlog::warn("ModbusClient: request id={} not found in queue", requestId);
        return;
    }
    const auto waitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - it->enqueueAt).count();
    spdlog::info("ModbusClient: finish request id={}, success={}, queue_wait={}ms, error='{}'",
                 requestId,
                 success,
                 waitMs,
                 error.toStdString());
    pendingRequests_.erase(it);
}

void ModbusClient::clearRuntimeState(bool clearPendingQueue) {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.clear();
    responseReady_ = false;
    lastError_.clear();
    if (clearPendingQueue) {
        std::lock_guard<std::mutex> pendingLock(pendingMutex_);
        pendingRequests_.clear();
    }
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
    if (channel_) {
        channel_->setReadHandler({});
        channel_->setErrorHandler({});
    }
    disconnect();
}

bool ModbusClient::connect() {
    if (!channel_) return false;
    
    // 应用超时配置
    io::Timeouts timeouts;
    timeouts.readMs = config_.timeoutMs;
    timeouts.writeMs = config_.timeoutMs;
    channel_->setTimeouts(timeouts);
    
    const bool opened = channel_->open();
    if (opened) {
        aborted_ = false;
        clearRuntimeState(false);
        transitionTo(RequestState::Idle, "connect");
    }
    return opened;
}

void ModbusClient::disconnect() {
    aborted_ = true;
    if (channel_) {
        channel_->close();
    }
    clearRuntimeState(true);
    aborted_ = false;
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
    aborted_ = false;
    
    int retries = config_.retries;
    ModbusResponse lastResponse = ModbusResponse::Error("Unknown error");
    const int requestId = enqueuePendingRequest(request, slaveId);
    transitionTo(RequestState::Idle, "request-start");

    for (int i = 0; i <= retries; ++i) {
        if (aborted_) {
            transitionTo(RequestState::Aborted, "request-aborted-before-send");
            finishPendingRequest(requestId, false, "Aborted");
            return ModbusResponse::Error("Aborted");
        }

        lastResponse = sendRequestInternal(request, slaveId);
        if (lastResponse.isSuccess) {
            transitionTo(RequestState::Completed, "request-success");
            finishPendingRequest(requestId, true, QString());
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
    finishPendingRequest(requestId, false, lastResponse.error);
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
    int droppedInvalidBytes = 0;
    constexpr int kMaxDroppedInvalidBytes = 256;

    spdlog::info("ModbusClient: Entering wait loop, deadline in {}ms", config_.timeoutMs);
    
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!responseReady_ && lastError_.isEmpty() && !aborted_.load()) {
            const bool notified = cv_.wait_until(lock, deadline, [this]() {
                return aborted_.load() || responseReady_ || !lastError_.isEmpty();
            });
            if (!notified) {
                transitionTo(RequestState::Failed, "timeout");
                return ModbusResponse::Error("Timeout");
            }
        }
        if (aborted_) {
            spdlog::info("ModbusClient: Aborted during wait");
            transitionTo(RequestState::Aborted, "aborted-during-wait");
            return ModbusResponse::Error("Aborted");
        }
        if (!lastError_.isEmpty()) {
            QString err = lastError_;
            lastError_.clear();
            transitionTo(RequestState::Failed, "channel-error");
            return ModbusResponse::Error(err);
        }

        responseReady_ = false;

        while (true) {
            int integrity = transport_->checkIntegrity(buffer_);
            if (integrity > 0) {
                QByteArray frame = buffer_.left(integrity);
                buffer_.remove(0, integrity);
                lock.unlock();
                auto parseResult = transport_->parseResponse(frame);
                if (parseResult.status == transport::ParseResponseStatus::Ok && parseResult.pdu) {
                    const auto& pdu = *parseResult.pdu;
                    if (pdu.isException()) {
                        transitionTo(RequestState::Failed, "modbus-exception");
                        return ModbusResponse::Error(QString("Modbus Exception: %1").arg(static_cast<int>(pdu.exceptionCode())));
                    }
                    if (pdu.originalFunctionCode() != request.functionCode()) {
                        lock.lock();
                        continue;
                    }
                    auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
                    transitionTo(RequestState::Completed, "response-parsed");
                    return ModbusResponse::Success(pdu, static_cast<int>(rttMs));
                }
                if (parseResult.status == transport::ParseResponseStatus::Unmatched) {
                    lock.lock();
                    continue;
                }
                if (parseResult.status == transport::ParseResponseStatus::Invalid) {
                    transitionTo(RequestState::Failed, "response-parse-failed");
                    return ModbusResponse::Error("Response parsing failed");
                }
                lock.lock();
                continue;
            } else if (integrity == -1) {
                if (!buffer_.isEmpty()) {
                    buffer_.remove(0, 1);
                    ++droppedInvalidBytes;
                }
                if (droppedInvalidBytes > kMaxDroppedInvalidBytes) {
                    transitionTo(RequestState::Failed, "too-many-invalid-bytes");
                    return ModbusResponse::Error("Too many invalid response bytes");
                }
                responseReady_ = !buffer_.isEmpty();
                continue;
            }
            break;
        }
        
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
