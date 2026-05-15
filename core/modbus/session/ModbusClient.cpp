/**
 * @file ModbusClient.cpp
 * @brief Implementation of ModbusClient.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusClient.h"
#include "ExceptionInterpreter.h"
#include "RequestValidator.h"
#include "AppConstants.h"
#include "logging/Logger.h"
#include "../base/ModbusProtocolChecks.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <random>
#include <QtEndian>
#include <QCoreApplication>
#include <QEventLoop>

namespace modbus::session {

namespace {

QString trClient(const char* text)
{
    return QCoreApplication::translate("ModbusClient", text);
}

bool isBroadcastWriteFunction(base::FunctionCode functionCode)
{
    using base::FunctionCode;
    switch (functionCode) {
    case FunctionCode::WriteSingleCoil:
    case FunctionCode::WriteSingleRegister:
    case FunctionCode::WriteMultipleCoils:
    case FunctionCode::WriteMultipleRegisters:
        return true;
    default:
        return false;
    }
}

constexpr auto kQtWaitSlice = std::chrono::milliseconds(5);

void pumpCurrentThreadEvents(std::chrono::milliseconds maxTime)
{
    if (!QCoreApplication::instance()) {
        return;
    }
    QCoreApplication::processEvents(QEventLoop::AllEvents, static_cast<int>(maxTime.count()));
}

} // namespace

const char* ModbusClient::toString(ConnectionState state) {
    switch (state) {
    case ConnectionState::Disconnected: return "Disconnected";
    case ConnectionState::Connecting: return "Connecting";
    case ConnectionState::Connected: return "Connected";
    case ConnectionState::Reconnecting: return "Reconnecting";
    case ConnectionState::Disconnecting: return "Disconnecting";
    case ConnectionState::Failed: return "Failed";
    }
    return "Unknown";
}

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

void ModbusClient::transitionConnectionState(ConnectionState newState, const char* reason) {
    ConnectionState oldState = connectionState_.exchange(newState);
    if (oldState == newState) {
        return;
    }
    MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: connection {} -> {} ({})",
                              toString(oldState),
                              toString(newState),
                              reason);
}

void ModbusClient::transitionTo(RequestState newState, const char* reason) {
    RequestState oldState = requestState_.exchange(newState);
    if (oldState == newState) {
        return;
    }
    MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: state {} -> {} ({})",
                              toString(oldState),
                              toString(newState),
                              reason);
}

ModbusClient::ConnectionState ModbusClient::connectionState() const {
    return connectionState_.load();
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
    MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: enqueue request id={}, fc={}, slave={}, queue={}",
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
    MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: finish request id={}, success={}, queue_wait={}ms, error='{}'",
                              requestId,
                              success,
                              waitMs,
                              error.toStdString());
    pendingRequests_.erase(it);
}

void ModbusClient::clearRuntimeState(bool clearPendingQueue) {
    std::lock_guard<std::mutex> lock(mutex_);
    frameExtractor_.reset();
    responseReady_ = false;
    writeDrained_ = true;
    lastChannelError_.clear();
    lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
    nextRtuSendAllowedAt_ = std::chrono::steady_clock::time_point{};
    if (clearPendingQueue) {
        std::lock_guard<std::mutex> pendingLock(pendingMutex_);
        pendingRequests_.clear();
    }
    if (transport_) {
        transport_->resetPendingState();
    }
}

ModbusClient::ModbusClient(std::shared_ptr<io::IChannel> channel, 
                           std::shared_ptr<transport::ITransport> transport)
    : channel_(std::move(channel)), transport_(std::move(transport))
    , frameExtractor_(base::ModbusMode::TCP, 9600)
    , retryStrategy_(RetryStrategy::Config{}) {
    
    // 设置 Channel 回调
    channel_->setReadHandler([this](QByteArrayView data) {
        onDataReceived(data);
    });
    
    channel_->setErrorHandler([this](const QString& error) {
        onChannelError(error);
    });

    channel_->setWriteDrainedHandler([this]() {
        std::lock_guard<std::mutex> lock(mutex_);
        writeDrained_ = true;
        lastWriteDrainedAt_ = std::chrono::steady_clock::now();
        cv_.notify_one();
    });
    stateHandlerId_ = channel_->addStateHandler([this](io::ChannelState) {
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_all();
    });
}

ModbusClient::~ModbusClient() {
    abort();
    if (channel_) {
        channel_->setReadHandler({});
        channel_->setErrorHandler({});
        channel_->setWriteDrainedHandler({});
        channel_->removeStateHandler(stateHandlerId_);
    }
    clearRuntimeState(true);
}

bool ModbusClient::connect() {
    if (!channel_) return false;

    aborted_ = false;
    const bool connected = ensureConnected(config_.autoReconnect);
    if (connected) {
        clearRuntimeState(false);
        transitionTo(RequestState::Idle, "connect");
    }
    return connected;
}

void ModbusClient::disconnect() {
    aborted_ = true;
    transitionConnectionState(ConnectionState::Disconnecting, "disconnect");
    if (channel_) {
        channel_->close();
    }
    clearRuntimeState(true);
    aborted_ = false;
    transitionConnectionState(ConnectionState::Disconnected, "disconnect-complete");
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

QString ModbusClient::lastChannelError() {
    std::lock_guard<std::mutex> lock(mutex_);
    return lastChannelError_;
}

void ModbusClient::setConfig(const base::ModbusConfig& config) {
    config_ = config;
    frameExtractor_.setConfig(config);
    retryStrategy_.reconfigure(RetryStrategy::Config{
        config.retries,
        config.retryIntervalMs,
        config.maxRetryIntervalMs,
        config.retryBackoffFactor,
        config.retryJitterPercent});
    if (isConnected()) {
        // 如果已经连接，动态更新超时
        io::Timeouts timeouts;
        timeouts.readMs = config_.timeoutMs;
        timeouts.writeMs = config_.timeoutMs;
        channel_->setTimeouts(timeouts);
    }
}

bool ModbusClient::ensureConnected(bool allowReconnect) {
    if (!channel_) {
        std::lock_guard<std::mutex> lock(mutex_);
        lastChannelError_ = trClient("No channel attached");
        transitionConnectionState(ConnectionState::Failed, "no-channel");
        return false;
    }

    io::Timeouts timeouts;
    timeouts.readMs = config_.timeoutMs;
    timeouts.writeMs = config_.timeoutMs;
    channel_->setTimeouts(timeouts);

    if (channel_->isOpen()) {
        transitionConnectionState(ConnectionState::Connected, "already-open");
        return true;
    }

    const int attempts = allowReconnect ? std::max(1, config_.retries + 1) : 1;
    QString connectError;
    for (int attempt = 0; attempt < attempts; ++attempt) {
        if (aborted_.load()) {
            std::lock_guard<std::mutex> lock(mutex_);
            lastChannelError_ = trClient("Aborted");
            transitionConnectionState(ConnectionState::Failed, "connect-aborted");
            return false;
        }

        transitionConnectionState(attempt == 0 ? ConnectionState::Connecting : ConnectionState::Reconnecting,
                                  attempt == 0 ? "connect-attempt" : "reconnect-attempt");
        {
            std::lock_guard<std::mutex> lock(mutex_);
            lastChannelError_.clear();
            responseReady_ = false;
        }

        if (!channel_->open()) {
            connectError = trClient("Failed to dispatch channel open");
        } else {
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(config_.timeoutMs);
            if (waitForChannelState(io::ChannelState::Open, deadline, &connectError)) {
                transitionConnectionState(ConnectionState::Connected, "connect-success");
                return true;
            }
        }

        channel_->close();
        transitionConnectionState(ConnectionState::Failed, "connect-failed");
        if (attempt + 1 >= attempts) {
            break;
        }

        BackoffCalculator::Config reconnectBackoffCfg;
        reconnectBackoffCfg.baseIntervalMs = config_.reconnectBaseMs;
        reconnectBackoffCfg.maxIntervalMs = config_.reconnectMaxMs;
        reconnectBackoffCfg.backoffFactor = config_.retryBackoffFactor;
        reconnectBackoffCfg.jitterPercent = config_.retryJitterPercent;
        BackoffCalculator reconnectBackoff(reconnectBackoffCfg);
        const int reconnectDelayMs = reconnectBackoff.calculateMs(attempt);
        if (!waitForAbortableDelay(std::chrono::milliseconds(reconnectDelayMs))) {
            std::lock_guard<std::mutex> lock(mutex_);
            lastChannelError_ = trClient("Aborted");
            transitionConnectionState(ConnectionState::Failed, "reconnect-aborted");
            return false;
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    lastChannelError_ = connectError.isEmpty() ? trClient("Connect timeout") : connectError;
    spdlog::warn("ModbusClient: connect failed target={}:{} reason={} channelState={}",
                 config_.ipAddress.toStdString(),
                 config_.port,
                 lastChannelError_.toStdString(),
                 static_cast<int>(channel_->state()));
    return false;
}

bool ModbusClient::waitForChannelState(io::ChannelState expectedState,
                                       std::chrono::steady_clock::time_point deadline,
                                       QString* errorOut) {
    while (std::chrono::steady_clock::now() < deadline) {
        if (aborted_.load()) {
            if (errorOut) {
                *errorOut = trClient("Aborted");
            }
            return false;
        }
        if (channel_->state() == expectedState || (expectedState == io::ChannelState::Open && channel_->isOpen())) {
            return true;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!lastChannelError_.isEmpty()) {
                if (errorOut) {
                    *errorOut = lastChannelError_;
                }
                lastChannelError_.clear();
                return false;
            }
        }
        if (channel_->state() == io::ChannelState::Error) {
            if (errorOut && errorOut->isEmpty()) {
                *errorOut = trClient("Channel entered error state");
            }
            return false;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        pumpCurrentThreadEvents(slice);

        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, slice, [this, expectedState]() {
                return aborted_.load() ||
                    !lastChannelError_.isEmpty() ||
                    channel_->state() == io::ChannelState::Error ||
                    channel_->state() == expectedState ||
                    (expectedState == io::ChannelState::Open && channel_->isOpen());
            })) {
            continue;
        }
    }

    if (channel_->state() == expectedState || (expectedState == io::ChannelState::Open && channel_->isOpen())) {
        return true;
    }
    if (errorOut && errorOut->isEmpty()) {
        *errorOut = trClient("Connect timeout");
    }
    return false;
}

bool ModbusClient::waitForWriteDrain(std::chrono::steady_clock::time_point deadline,
                                     std::chrono::steady_clock::time_point* drainedAt) {
    while (std::chrono::steady_clock::now() < deadline) {
        if (aborted_.load()) {
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (writeDrained_) {
                if (drainedAt) {
                    *drainedAt = (lastWriteDrainedAt_ == std::chrono::steady_clock::time_point{})
                        ? std::chrono::steady_clock::now()
                        : lastWriteDrainedAt_;
                }
                return true;
            }
            if (!lastChannelError_.isEmpty()) {
                return false;
            }
        }
        if (channel_->state() == io::ChannelState::Error) {
            return false;
        }

        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        pumpCurrentThreadEvents(slice);

        std::unique_lock<std::mutex> lock(mutex_);
        if (!cv_.wait_for(lock, slice, [this]() {
                return aborted_.load() || writeDrained_ || !lastChannelError_.isEmpty() ||
                    channel_->state() == io::ChannelState::Error;
            })) {
            continue;
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (writeDrained_) {
        if (drainedAt) {
            *drainedAt = (lastWriteDrainedAt_ == std::chrono::steady_clock::time_point{})
                ? std::chrono::steady_clock::now()
                : lastWriteDrainedAt_;
        }
        return true;
    }
    return false;
}

bool ModbusClient::waitForEventOrTimeout(std::chrono::steady_clock::time_point deadline) {
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (aborted_.load() || responseReady_ || !lastChannelError_.isEmpty() ||
                channel_->state() == io::ChannelState::Error) {
                return true;
            }
        }

        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        pumpCurrentThreadEvents(slice);

        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, slice, [this]() {
                return aborted_.load() || responseReady_ || !lastChannelError_.isEmpty() ||
                    channel_->state() == io::ChannelState::Error;
            })) {
            return true;
        }
    }
    return false;
}

ModbusResponse ModbusClient::sendRequest(const base::Pdu& request, int slaveId) {
    std::lock_guard<std::recursive_mutex> lock(requestMutex_);
    aborted_ = false;

    retryStrategy_.reset();
    ModbusResponse lastResponse = ModbusResponse::Error(trClient("Unknown error"));
    const int requestId = enqueuePendingRequest(request, slaveId);
    transitionTo(RequestState::Idle, "request-start");

    for (;;) {
        if (aborted_) {
            transitionTo(RequestState::Aborted, "request-aborted-before-send");
            finishPendingRequest(requestId, false, "Aborted");
            return ModbusResponse::Error(trClient("Aborted"), retryStrategy_.attemptCount());
        }

        lastResponse = sendRequestInternal(request, slaveId);
        lastResponse.attemptCount = retryStrategy_.attemptCount() + 1;
        lastResponse.retryCount = retryStrategy_.attemptCount();
        if (lastResponse.isSuccess) {
            transitionTo(RequestState::Completed, "request-success");
            finishPendingRequest(requestId, true, QString());
            return lastResponse;
        }
        
        retryStrategy_.recordAttempt();
        if (retryStrategy_.shouldRetry() && !aborted_) {
            transitionTo(RequestState::Failed, "request-retry");
            const auto retryDelay = retryStrategy_.nextWait();
            spdlog::warn("Request failed, retrying... ({}/{}) Error: {}", 
                         retryStrategy_.attemptCount(),
                         config_.retries,
                         lastResponse.error.toStdString());
            if (!waitForAbortableDelay(retryDelay)) {
                transitionTo(RequestState::Aborted, "request-aborted-during-backoff");
                finishPendingRequest(requestId, false, "Aborted");
                return ModbusResponse::Error(trClient("Aborted"), retryStrategy_.attemptCount());
            }
        } else {
            break;
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
        waitForRtuInterFrameDelay();
        {
            std::lock_guard<std::mutex> stateLock(mutex_);
            lastChannelError_.clear();
            writeDrained_ = config_.mode != base::ModbusMode::RTU;
            lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
        }
        if (channel_->write(data)) {
            updateRtuSendWindow(data.size());
            if (config_.mode == base::ModbusMode::RTU) {
                const auto writeDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(config_.timeoutMs);
                waitForWriteDrain(writeDeadline, nullptr);
            }
        }
    }
}

ModbusResponse ModbusClient::sendRequestInternal(const base::Pdu& request, int slaveId) {
    if (!ensureConnected(config_.autoReconnect)) {
        transitionTo(RequestState::Failed, "not-connected");
        std::lock_guard<std::mutex> lock(mutex_);
        return ModbusResponse::Error(lastChannelError_.isEmpty() ? trClient("Not connected") : lastChannelError_);
    }

    const int targetSlaveId = (slaveId == -1) ? config_.slaveId : slaveId;
    const auto validationResult = requestValidator_.validate(request, targetSlaveId, config_.mode);
    if (!validationResult.valid) {
        transitionTo(RequestState::Failed, "request-validation-failed");
        return ModbusResponse::Error(validationResult.error);
    }

    // 1. 清理旧缓冲区和状态
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (aborted_) {
            transitionTo(RequestState::Aborted, "aborted-before-build");
            return ModbusResponse::Error(trClient("Aborted"));
        }
        frameExtractor_.reset();
        responseReady_ = false;
        writeDrained_ = config_.mode != base::ModbusMode::RTU;
        lastChannelError_.clear();
        lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
    }
    transport_->resetPendingState();

    // 2. 构建 ADU
    if (isRtuBroadcastRequest(targetSlaveId, request.functionCode()) &&
        !isBroadcastWriteFunction(request.functionCode())) {
        transitionTo(RequestState::Failed, "invalid-rtu-broadcast-function");
        return ModbusResponse::Error(trClient("RTU broadcast only supports write function codes"));
    }
    QByteArray adu = transport_->buildRequest(request, targetSlaveId);
    waitForRtuInterFrameDelay();

    // 3. 发送数据
    if (!channel_->write(adu)) {
        transitionTo(RequestState::Failed, "write-failed");
        return ModbusResponse::Error(trClient("Write failed"));
    }
    updateRtuSendWindow(adu.size());
    transitionTo(RequestState::Sending, "write-success");
    auto start = std::chrono::steady_clock::now();

    if (config_.mode == base::ModbusMode::RTU) {
        std::chrono::steady_clock::time_point drainedAt{};
        const auto writeDeadline = start + std::chrono::milliseconds(config_.timeoutMs);
        if (!waitForWriteDrain(writeDeadline, &drainedAt)) {
            transitionTo(RequestState::Failed, "write-drain-timeout");
            std::lock_guard<std::mutex> lock(mutex_);
            const QString error = lastChannelError_.isEmpty()
                ? trClient("Write drain timeout")
                : lastChannelError_;
            lastChannelError_.clear();
            return ModbusResponse::Error(error);
        }
        if (drainedAt != std::chrono::steady_clock::time_point{}) {
            start = drainedAt;
        }
    }

    if (!shouldWaitForResponse(targetSlaveId, request.functionCode())) {
        transitionTo(RequestState::Completed, "broadcast-write-no-response");
        return ModbusResponse::NoResponseExpected(base::Pdu(request.functionCode(), request.data()));
    }

    // 4. 等待响应
    // 使用绝对时间作为截止日期，避免循环中相对时间重置
    auto deadline = start + std::chrono::milliseconds(config_.timeoutMs);
    if (config_.mode == base::ModbusMode::RTU) {
        deadline += FrameExtractor::calculateInterFrameDelay(config_);
    }
    transitionTo(RequestState::Waiting, "wait-response");
    MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: Entering wait loop, deadline in {}ms", config_.timeoutMs);
    
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        const auto now = std::chrono::steady_clock::now();
        const bool rtuFrameReady = frameExtractor_.isRtuFrameReadyToParse(now);
        if (!rtuFrameReady && !responseReady_ && lastChannelError_.isEmpty() && !aborted_.load()) {
            lock.unlock();
            const bool stillWaiting = waitForEventOrTimeout(deadline);
            lock.lock();
            if (!stillWaiting && std::chrono::steady_clock::now() >= deadline) {
                transitionTo(RequestState::Failed, "timeout");
                spdlog::warn("ModbusClient: request timeout slave={} fc={} timeoutMs={}",
                             slaveId, static_cast<int>(request.functionCode()), config_.timeoutMs);
                return ModbusResponse::Error(trClient("Timeout"));
            }
            continue;
        }
        if (!rtuFrameReady && config_.mode == base::ModbusMode::RTU && frameExtractor_.bufferSize() > 0) {
            responseReady_ = false;
            const auto frameDeadline = std::min(deadline, frameExtractor_.nextRtuFrameBoundary());
            lock.unlock();
            const bool stillWaiting = waitForEventOrTimeout(frameDeadline);
            lock.lock();
            if (!stillWaiting && std::chrono::steady_clock::now() >= deadline) {
                transitionTo(RequestState::Failed, "timeout");
                spdlog::warn("ModbusClient: RTU frame wait timeout slave={} fc={} timeoutMs={}",
                             slaveId, static_cast<int>(request.functionCode()), config_.timeoutMs);
                return ModbusResponse::Error(trClient("Timeout"));
            }
            continue;
        }
        if (aborted_) {
            MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: Aborted during wait");
            transitionTo(RequestState::Aborted, "aborted-during-wait");
            return ModbusResponse::Error(trClient("Aborted"));
        }
        if (!lastChannelError_.isEmpty()) {
            QString err = lastChannelError_;
            lastChannelError_.clear();
            transitionTo(RequestState::Failed, "channel-error");
            return ModbusResponse::Error(err);
        }

        if (frameExtractor_.hasExceededDropLimit()) {
            transitionTo(RequestState::Failed, "too-many-invalid-bytes");
            return ModbusResponse::Error(trClient("Too many invalid response bytes"));
        }

        responseReady_ = false;

        while (true) {
            if (config_.mode == base::ModbusMode::RTU) {
                auto frameOpt = frameExtractor_.tryPopRtuResponseFrame(now);
                if (frameOpt) {
                    lock.unlock();
                    auto parseResult = transport_->parseResponse(*frameOpt);
                    if (parseResult.status == transport::ParseResponseStatus::Ok && parseResult.pdu) {
                        const auto& pdu = *parseResult.pdu;
                        if (pdu.isException()) {
                            transitionTo(RequestState::Failed, "modbus-exception");
                            const QString exceptionMessage = ExceptionInterpreter::buildMessage(
                                slaveId,
                                request.functionCode(),
                                pdu.exceptionCode());
                            spdlog::warn("ModbusClient: {}", exceptionMessage.toStdString());
                            return ModbusResponse::Error(exceptionMessage);
                        }
                        if (pdu.originalFunctionCode() != request.functionCode()) {
                            lock.lock();
                            continue;
                        }
                        const QString responseValidationError = base::validateResponsePdu(request, pdu);
                        if (!responseValidationError.isEmpty()) {
                            transitionTo(RequestState::Failed, "response-validation-failed");
                            return ModbusResponse::Error(responseValidationError);
                        }
                        auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - start).count();
                        transitionTo(RequestState::Completed, "response-parsed");
                        return ModbusResponse::Success(pdu, static_cast<int>(rttMs));
                    }
                    if (parseResult.status == transport::ParseResponseStatus::Unmatched) {
                        lock.lock();
                        continue;
                    }
                    transitionTo(RequestState::Failed, "response-parse-failed");
                    return ModbusResponse::Error(trClient("Response parsing failed"));
                }
                break;
            }

            auto frameOpt = frameExtractor_.popFrame();
            if (frameOpt) {
                lock.unlock();
                auto parseResult = transport_->parseResponse(*frameOpt);
                if (parseResult.status == transport::ParseResponseStatus::Ok && parseResult.pdu) {
                    const auto& pdu = *parseResult.pdu;
                    if (pdu.isException()) {
                        transitionTo(RequestState::Failed, "modbus-exception");
                        const QString exceptionMessage = ExceptionInterpreter::buildMessage(
                            slaveId,
                            request.functionCode(),
                            pdu.exceptionCode());
                        spdlog::warn("ModbusClient: {}", exceptionMessage.toStdString());
                        return ModbusResponse::Error(exceptionMessage);
                    }
                    if (pdu.originalFunctionCode() != request.functionCode()) {
                        lock.lock();
                        continue;
                    }
                    const QString responseValidationError = base::validateResponsePdu(request, pdu);
                    if (!responseValidationError.isEmpty()) {
                        transitionTo(RequestState::Failed, "response-validation-failed");
                        return ModbusResponse::Error(responseValidationError);
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
                    return ModbusResponse::Error(trClient("Response parsing failed"));
                }
                lock.lock();
                continue;
            }
            if (config_.mode == base::ModbusMode::RTU && frameExtractor_.bufferSize() > 0 &&
                frameExtractor_.isRtuFrameReadyToParse(std::chrono::steady_clock::now())) {
                transitionTo(RequestState::Failed, "incomplete-rtu-frame-after-gap");
                return ModbusResponse::Error(trClient("Incomplete RTU frame after inter-frame silence"));
            }
            break;
        }
        
        if (std::chrono::steady_clock::now() >= deadline) {
            transitionTo(RequestState::Failed, "timeout-full-packet");
            spdlog::warn("ModbusClient: full packet wait timeout slave={} fc={} timeoutMs={}",
                         slaveId, static_cast<int>(request.functionCode()), config_.timeoutMs);
            return ModbusResponse::Error(trClient("Timeout while waiting for full packet"));
        }
    }
}

bool ModbusClient::isRtuBroadcastRequest(int slaveId, base::FunctionCode functionCode) const {
    Q_UNUSED(functionCode);
    return config_.mode == base::ModbusMode::RTU && slaveId == 0;
}

bool ModbusClient::shouldWaitForResponse(int slaveId, base::FunctionCode functionCode) const {
    if (!isRtuBroadcastRequest(slaveId, functionCode)) {
        return true;
    }
    return false;
}

void ModbusClient::waitForRtuInterFrameDelay() {
    if (config_.mode != base::ModbusMode::RTU) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    if (nextRtuSendAllowedAt_ > now) {
        waitForAbortableDelay(nextRtuSendAllowedAt_ - now);
    }
}

bool ModbusClient::waitForAbortableDelay(std::chrono::steady_clock::duration delay) {
    if (delay <= std::chrono::steady_clock::duration::zero()) {
        return !aborted_.load();
    }

    const auto deadline = std::chrono::steady_clock::now() + delay;
    while (!aborted_.load() && std::chrono::steady_clock::now() < deadline) {
        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        pumpCurrentThreadEvents(slice);

        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, slice, [this]() {
            return aborted_.load();
        });
    }
    return !aborted_.load();
}

void ModbusClient::updateRtuSendWindow(qsizetype frameBytes) {
    if (config_.mode != base::ModbusMode::RTU) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    nextRtuSendAllowedAt_ = now + FrameExtractor::calculateFrameDuration(config_, frameBytes) + FrameExtractor::calculateInterFrameDelay(config_);
}

void ModbusClient::onDataReceived(QByteArrayView data) {
    std::lock_guard<std::mutex> lock(mutex_);
    MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: Data received, size={}, notifying loop", data.size());

    frameExtractor_.feed(data);
    responseReady_ = frameExtractor_.hasCompleteFrame();
    cv_.notify_one();
}

void ModbusClient::onChannelError(const QString& error) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        lastChannelError_ = error;
    }
    spdlog::warn("ModbusClient: channel error forwarded: '{}' state={} connState={}",
                 error.toStdString(),
                 toString(requestState_.load()),
                 toString(connectionState_.load()));
    cv_.notify_one();
}

} // namespace modbus::session
