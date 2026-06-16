/**
 * @file RequestExecutor.cpp
 * @brief Implementation of RequestExecutor.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "RequestExecutor.h"
#include "ExceptionInterpreter.h"
#include "RequestValidator.h"
#include "AppConstants.h"
#include "infra/logging/Logger.h"
#include "../base/ModbusProtocolChecks.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <tuple>
#include <QtEndian>
#include <QCoreApplication>

namespace modbus::session {
namespace {

    QString trReq(const char* text) {
        return QObject::tr(text);
    }

    bool isBroadcastWriteFunction(base::FunctionCode functionCode) {
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

} // namespace

RequestExecutor::RequestExecutor(io::IChannel* channel,
                                 transport::ITransport* transport,
                                 FrameExtractor* frameExtractor,
                                 FlowController* flowController,
                                 RetryStrategy* retryStrategy,
                                 RequestValidator* requestValidator,
                                 ConnectionStateMachine* connStateMachine,
                                 RequestStateMachine* reqStateMachine,
                                 TimeoutController* timeoutController,
                                 ConnectionManager* connectionManager,
                                 const base::ModbusConfig* config,
                                 std::mutex& mutex,
                                 std::condition_variable& cv,
                                 std::atomic<bool>& aborted,
                                 std::mutex& pendingMutex,
                                 std::deque<PendingRequest>& pendingRequests,
                                 int& nextRequestId)
    : channel_(channel)
    , transport_(transport)
    , frameExtractor_(frameExtractor)
    , flowController_(flowController)
    , retryStrategy_(retryStrategy)
    , requestValidator_(requestValidator)
    , connStateMachine_(connStateMachine)
    , reqStateMachine_(reqStateMachine)
    , timeoutController_(timeoutController)
    , connectionManager_(connectionManager)
    , config_(config)
    , mutex_(mutex)
    , cv_(cv)
    , aborted_(aborted)
    , pendingMutex_(pendingMutex)
    , pendingRequests_(pendingRequests)
    , nextRequestId_(nextRequestId) {}

// --- Public API ---

ModbusResponse RequestExecutor::execute(const base::Pdu& request, int slaveId) {
    assert(!requestLocked_.exchange(true, std::memory_order_acquire)
           && "requestMutex re-entry detected in sendRequest()");
    RequestLockGuard unlockGuard(requestLocked_);
    std::lock_guard<std::mutex> lock(requestMutex_);
    aborted_ = false;

    retryStrategy_->reset();
    ModbusResponse lastResponse = ModbusResponse::Error(trReq("Unknown error"));
    const int requestId = enqueuePendingRequest(request, slaveId);
    reqStateMachine_->tryTransition(RequestStateMachine::State::Idle, "request-start");

    for (;;) {
        if (aborted_) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                            "request-aborted-before-send");
            finishPendingRequest(requestId, false, "Aborted");
            return ModbusResponse::Error(trReq("Aborted"), retryStrategy_->attemptCount());
        }

        lastResponse = sendRequestInternal(request, slaveId);
        lastResponse.attemptCount = retryStrategy_->attemptCount() + 1;
        lastResponse.retryCount = retryStrategy_->attemptCount();
        if (lastResponse.isSuccess) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Completed,
                                            "request-success");
            finishPendingRequest(requestId, true, QString());
            return lastResponse;
        }

        retryStrategy_->recordAttempt();
        if (retryStrategy_->shouldRetry() && !aborted_) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                            "request-retry");
            const auto retryDelay = retryStrategy_->nextWait();
            spdlog::warn("Request failed, retrying... ({}/{}) Error: {}",
                         retryStrategy_->attemptCount(),
                         config_->retries,
                         lastResponse.error.toStdString());
            if (!timeoutController_->waitForAbortableDelay(retryDelay)) {
                reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                                "request-aborted-during-backoff");
                finishPendingRequest(requestId, false, "Aborted");
                return ModbusResponse::Error(trReq("Aborted"), retryStrategy_->attemptCount());
            }
        } else {
            break;
        }
    }
    if (aborted_) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                        "request-aborted-after-retries");
    } else {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                        "request-failed");
    }
    finishPendingRequest(requestId, false, lastResponse.error);
    return lastResponse;
}

void RequestExecutor::sendRaw(const QByteArray& data) {
    assert(!requestLocked_.exchange(true, std::memory_order_acquire)
           && "requestMutex re-entry detected in sendRaw()");
    RequestLockGuard unlockGuard(requestLocked_);
    std::lock_guard<std::mutex> lock(requestMutex_);
    if (connectionManager_->isConnected()) {
        if (!flowController_->isRtuSendWindowOpen(std::chrono::steady_clock::now())) {
            timeoutController_->waitForAbortableDelay(
                flowController_->rtuSendWindowOpensAt() - std::chrono::steady_clock::now());
        }
        {
            std::lock_guard<std::mutex> stateLock(mutex_);
            if (config_->mode == base::ModbusMode::RTU) {
                flowController_->markWritePending();
            }
        }
        if (channel_->write(data)) {
            flowController_->updateRtuSendWindow(data.size(), *config_);
            if (config_->mode == base::ModbusMode::RTU) {
                const auto writeDeadline = std::chrono::steady_clock::now()
                    + std::chrono::milliseconds(config_->timeoutMs);
                waitForWriteDrain(writeDeadline, nullptr);
            }
        }
    }
}

void RequestExecutor::abort() {
    spdlog::info("ModbusClient: Abort requested");
    aborted_ = true;
    const auto current = reqStateMachine_->currentState();
    if (current != RequestStateMachine::State::Completed
        && current != RequestStateMachine::State::Failed) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted, "abort");
    }
    cv_.notify_all();
}

// --- Callbacks ---

void RequestExecutor::onDataReceived(QByteArrayView data) {
    std::lock_guard<std::mutex> lock(mutex_);
    MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: Data received, size={}, notifying loop", data.size());

    frameExtractor_->feed(data);
    responseReady_ = frameExtractor_->hasCompleteFrame()
        || (config_->mode == base::ModbusMode::RTU && frameExtractor_->bufferSize() > 0);
    cv_.notify_one();
}

void RequestExecutor::onChannelError(const QString& error) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connectionManager_->setError(error);
    }
    spdlog::warn("ModbusClient: channel error forwarded: '{}' state={} connState={}",
                 error.toStdString(),
                 static_cast<int>(channel_->state()),
                 static_cast<int>(connStateMachine_->currentState()));
    cv_.notify_one();
}

void RequestExecutor::resetState(bool clearPendingQueue) {
    std::lock_guard<std::mutex> lock(mutex_);
    responseReady_ = false;
    dupeTracker_.clear();
    if (clearPendingQueue) {
        std::lock_guard<std::mutex> pendingLock(pendingMutex_);
        pendingRequests_.clear();
    }
}

// --- Private: Request flow ---

ModbusResponse RequestExecutor::sendRequestInternal(const base::Pdu& request, int slaveId) {
    if (!connectionManager_->ensureConnected(config_->autoReconnect)) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "not-connected");
        std::lock_guard<std::mutex> lock(mutex_);
        return ModbusResponse::Error(
            connectionManager_->lastChannelError().isEmpty()
                ? trReq("Not connected")
                : connectionManager_->lastChannelError());
    }

    const int targetSlaveId = (slaveId == -1) ? config_->slaveId : slaveId;
    const auto validationResult = requestValidator_->validate(request, targetSlaveId, config_->mode);
    if (!validationResult.valid) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                        "request-validation-failed");
        return ModbusResponse::Error(validationResult.error);
    }

    // 1. Cleanup old buffers and state
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (aborted_) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                            "aborted-before-build");
            return ModbusResponse::Error(trReq("Aborted"));
        }
        frameExtractor_->reset();
        responseReady_ = false;
        if (config_->mode == base::ModbusMode::RTU) {
            flowController_->markWritePending();
        }
        transport_->resetPendingState();
    }

    // 2. Build ADU
    if (isRtuBroadcastRequest(targetSlaveId, request.functionCode())
        && !isBroadcastWriteFunction(request.functionCode())) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                        "invalid-rtu-broadcast-function");
        return ModbusResponse::Error(
            trReq("RTU broadcast only supports write function codes"));
    }
    QByteArray adu = transport_->buildRequest(request, targetSlaveId);

    if (!flowController_->isRtuSendWindowOpen(std::chrono::steady_clock::now())) {
        timeoutController_->waitForAbortableDelay(
            flowController_->rtuSendWindowOpensAt() - std::chrono::steady_clock::now());
    }

    if (!channel_->write(adu)) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "write-failed");
        return ModbusResponse::Error(trReq("Write failed"));
    }
    flowController_->updateRtuSendWindow(adu.size(), *config_);
    reqStateMachine_->tryTransition(RequestStateMachine::State::Sending, "write-success");
    auto start = std::chrono::steady_clock::now();

    if (config_->mode == base::ModbusMode::RTU) {
        std::chrono::steady_clock::time_point drainedAt{};
        const auto writeDeadline = start + std::chrono::milliseconds(config_->timeoutMs);
        if (!waitForWriteDrain(writeDeadline, &drainedAt)) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                            "write-drain-timeout");
            std::lock_guard<std::mutex> lock(mutex_);
            const QString error = connectionManager_->lastChannelError().isEmpty()
                ? trReq("Write drain timeout")
                : connectionManager_->lastChannelError();
            return ModbusResponse::Error(error);
        }
        if (drainedAt != std::chrono::steady_clock::time_point{}) {
            start = drainedAt;
        }
    }

    if (!shouldWaitForResponse(targetSlaveId, request.functionCode())) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Completed,
                                        "broadcast-write-no-response");
        return ModbusResponse::NoResponseExpected(
            base::Pdu(request.functionCode(), request.data()));
    }

    // 4. Wait for response
    auto deadline = start + std::chrono::milliseconds(config_->timeoutMs);
    if (config_->mode == base::ModbusMode::RTU) {
        deadline += FrameExtractor::calculateInterFrameDelay(*config_);
    }
    reqStateMachine_->tryTransition(RequestStateMachine::State::Waiting, "wait-response");
    MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: Entering wait loop, deadline in {}ms",
                              config_->timeoutMs);

    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        const auto now = std::chrono::steady_clock::now();
        const bool rtuFrameReady = frameExtractor_->isRtuFrameReadyToParse(now);
        if (!rtuFrameReady && !responseReady_
            && connectionManager_->lastChannelError().isEmpty() && !aborted_.load()) {
            lock.unlock();
            const bool stillWaiting = waitForEventOrTimeout(deadline);
            lock.lock();
            if (!stillWaiting && std::chrono::steady_clock::now() >= deadline) {
                reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "timeout");
                spdlog::warn("ModbusClient: request timeout slave={} fc={} timeoutMs={}",
                             slaveId, static_cast<int>(request.functionCode()),
                             config_->timeoutMs);
                return ModbusResponse::Error(trReq("Timeout"));
            }
            continue;
        }
        if (!rtuFrameReady && config_->mode == base::ModbusMode::RTU
            && frameExtractor_->bufferSize() > 0) {
            responseReady_ = false;
            const auto frameDeadline = std::min(deadline,
                                                frameExtractor_->nextRtuFrameBoundary());
            lock.unlock();
            const bool stillWaiting = waitForEventOrTimeout(frameDeadline);
            lock.lock();
            if (!stillWaiting && std::chrono::steady_clock::now() >= deadline) {
                reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "timeout");
                spdlog::warn("ModbusClient: RTU frame wait timeout slave={} fc={} timeoutMs={}",
                             slaveId, static_cast<int>(request.functionCode()),
                             config_->timeoutMs);
                return ModbusResponse::Error(trReq("Timeout"));
            }
            continue;
        }
        if (aborted_) {
            MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: Aborted during wait");
            reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                            "aborted-during-wait");
            return ModbusResponse::Error(trReq("Aborted"));
        }
        {
            const QString chErr = connectionManager_->lastChannelError();
            if (!chErr.isEmpty()) {
                reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                                "channel-error");
                return ModbusResponse::Error(chErr);
            }
        }

        if (frameExtractor_->hasExceededDropLimit()) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                            "too-many-invalid-bytes");
            return ModbusResponse::Error(trReq("Too many invalid response bytes"));
        }

        responseReady_ = false;

        // Cleanup stale dupeTracker entries to prevent unbounded growth
        if (dupeTracker_.size() > 1000) {
            const auto cutoff = std::chrono::steady_clock::now() - std::chrono::seconds(5);
            for (auto it = dupeTracker_.begin(); it != dupeTracker_.end(); ) {
                if (it->second < cutoff) {
                    it = dupeTracker_.erase(it);
                } else {
                    ++it;
                }
            }
        }

        while (true) {
            if (config_->mode == base::ModbusMode::RTU) {
                auto frameOpt = frameExtractor_->tryPopRtuResponseFrame(now);
                if (frameOpt) {
                    lock.unlock();
                    auto parseResult = transport_->parseResponse(*frameOpt);
                    if (parseResult.status == transport::ParseResponseStatus::Ok
                        && parseResult.pdu) {
                        const auto& pdu = *parseResult.pdu;
                        if (pdu.isException()) {
                            reqStateMachine_->tryTransition(
                                RequestStateMachine::State::Failed, "modbus-exception");
                            return handleExceptionResponse(pdu, slaveId, request);
                        }
                        if (pdu.originalFunctionCode() != request.functionCode()) {
                            lock.lock();
                            continue;
                        }
                        const QString responseValidationError = base::validateResponsePdu(
                            request, pdu);
                        if (!responseValidationError.isEmpty()) {
                            reqStateMachine_->tryTransition(
                                RequestStateMachine::State::Failed,
                                "response-validation-failed");
                            return ModbusResponse::Error(responseValidationError);
                        }
                        auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - start).count();
                        reqStateMachine_->tryTransition(
                            RequestStateMachine::State::Completed, "response-parsed");
                        return ModbusResponse::Success(pdu, static_cast<int>(rttMs));
                    }
                    if (parseResult.status == transport::ParseResponseStatus::Unmatched) {
                        lock.lock();
                        continue;
                    }
                    reqStateMachine_->tryTransition(
                        RequestStateMachine::State::Failed, "response-parse-failed");
                    return ModbusResponse::Error(trReq("Response parsing failed"));
                }
                break;
            }

            auto frameOpt = frameExtractor_->popFrame();
            if (frameOpt) {
                lock.unlock();
                auto parseResult = transport_->parseResponse(*frameOpt);
                if (parseResult.status == transport::ParseResponseStatus::Ok
                    && parseResult.pdu) {
                    const auto& pdu = *parseResult.pdu;
                    if (pdu.isException()) {
                        reqStateMachine_->tryTransition(
                            RequestStateMachine::State::Failed, "modbus-exception");
                        return handleExceptionResponse(pdu, slaveId, request);
                    }
                    if (pdu.originalFunctionCode() != request.functionCode()) {
                        lock.lock();
                        continue;
                    }
                    const QString responseValidationError = base::validateResponsePdu(
                        request, pdu);
                    if (!responseValidationError.isEmpty()) {
                        reqStateMachine_->tryTransition(
                            RequestStateMachine::State::Failed,
                            "response-validation-failed");
                        return ModbusResponse::Error(responseValidationError);
                    }
                    auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - start).count();
                    reqStateMachine_->tryTransition(
                        RequestStateMachine::State::Completed, "response-parsed");
                    return ModbusResponse::Success(pdu, static_cast<int>(rttMs));
                }
                if (parseResult.status == transport::ParseResponseStatus::Unmatched) {
                    lock.lock();
                    continue;
                }
                if (parseResult.status == transport::ParseResponseStatus::Invalid) {
                    reqStateMachine_->tryTransition(
                        RequestStateMachine::State::Failed, "response-parse-failed");
                    return ModbusResponse::Error(trReq("Response parsing failed"));
                }
                lock.lock();
                continue;
            }
            if (config_->mode == base::ModbusMode::RTU
                && frameExtractor_->bufferSize() > 0
                && frameExtractor_->isRtuFrameReadyToParse(
                    std::chrono::steady_clock::now())) {
                reqStateMachine_->tryTransition(
                    RequestStateMachine::State::Failed,
                    "incomplete-rtu-frame-after-gap");
                return ModbusResponse::Error(
                    trReq("Incomplete RTU frame after inter-frame silence"));
            }
            break;
        }

        if (std::chrono::steady_clock::now() >= deadline) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                            "timeout-full-packet");
            spdlog::warn("ModbusClient: full packet wait timeout slave={} fc={} timeoutMs={}",
                         slaveId, static_cast<int>(request.functionCode()),
                         config_->timeoutMs);
            return ModbusResponse::Error(trReq("Timeout while waiting for full packet"));
        }
    }
}

ModbusResponse RequestExecutor::handleExceptionResponse(const base::Pdu& responsePdu, int slaveId,
                                                        const base::Pdu& requestPdu) {
    const QString exceptionMessage = ExceptionInterpreter::buildMessage(
        slaveId,
        requestPdu.functionCode(),
        responsePdu.exceptionCode());
    auto dupeKey = std::make_tuple(static_cast<uint8_t>(slaveId),
                                   static_cast<uint8_t>(requestPdu.functionCode()),
                                   static_cast<uint8_t>(responsePdu.exceptionCode()));
    auto now = std::chrono::steady_clock::now();
    auto it = dupeTracker_.find(dupeKey);
    if (it != dupeTracker_.end() && (now - it->second) < std::chrono::seconds(5)) {
        spdlog::debug("ModbusClient: Modbus exception response. "
                      "Slave={} FC=0x{:02X} Exception=0x{:02X} (duplicate within 5s)",
                      slaveId, static_cast<int>(requestPdu.functionCode()),
                      static_cast<int>(responsePdu.exceptionCode()));
    } else {
        spdlog::debug("ModbusClient: Modbus exception response. "
                      "Slave={} FC=0x{:02X} Exception=0x{:02X}",
                      slaveId, static_cast<int>(requestPdu.functionCode()),
                      static_cast<int>(responsePdu.exceptionCode()));
        dupeTracker_[dupeKey] = now;
    }
    return ModbusResponse::Error(exceptionMessage);
}

// --- Private: Helpers ---

bool RequestExecutor::isRtuBroadcastRequest(int slaveId,
                                            base::FunctionCode functionCode) const {
    Q_UNUSED(functionCode);
    return config_->mode == base::ModbusMode::RTU && slaveId == 0;
}

bool RequestExecutor::shouldWaitForResponse(int slaveId,
                                            base::FunctionCode functionCode) const {
    return !isRtuBroadcastRequest(slaveId, functionCode);
}

bool RequestExecutor::waitForWriteDrain(std::chrono::steady_clock::time_point deadline,
                                        std::chrono::steady_clock::time_point* drainedAt) {
    while (std::chrono::steady_clock::now() < deadline) {
        if (aborted_.load()) {
            return false;
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (flowController_->isWriteDrained()) {
                if (drainedAt) {
                    const auto da = flowController_->drainedAt();
                    *drainedAt = (da == std::chrono::steady_clock::time_point{})
                        ? std::chrono::steady_clock::now()
                        : da;
                }
                return true;
            }
            if (!connectionManager_->lastChannelError().isEmpty()) {
                return false;
            }
        }
        if (channel_->state() == io::ChannelState::Error) {
            return false;
        }

        timeoutController_->waitForCondition([this]() {
            return aborted_.load() || flowController_->isWriteDrained()
                || !connectionManager_->lastChannelError().isEmpty()
                || channel_->state() == io::ChannelState::Error;
        }, deadline);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (flowController_->isWriteDrained()) {
        if (drainedAt) {
            const auto da = flowController_->drainedAt();
            *drainedAt = (da == std::chrono::steady_clock::time_point{})
                ? std::chrono::steady_clock::now()
                : da;
        }
        return true;
    }
    return false;
}

bool RequestExecutor::waitForEventOrTimeout(std::chrono::steady_clock::time_point deadline) {
    while (std::chrono::steady_clock::now() < deadline) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (aborted_.load() || responseReady_
                || !connectionManager_->lastChannelError().isEmpty()
                || channel_->state() == io::ChannelState::Error) {
                return true;
            }
        }

        timeoutController_->waitForCondition([this]() {
            return aborted_.load() || responseReady_
                || !connectionManager_->lastChannelError().isEmpty()
                || channel_->state() == io::ChannelState::Error;
        }, deadline);
    }
    return false;
}

int RequestExecutor::enqueuePendingRequest(const base::Pdu& request, int slaveId) {
    std::lock_guard<std::mutex> pendingLock(pendingMutex_);
    PendingRequest item;
    item.requestId = nextRequestId_++;
    item.functionCode = request.functionCode();
    item.slaveId = (slaveId == -1) ? config_->slaveId : slaveId;
    item.timeoutMs = config_->timeoutMs;
    item.retries = config_->retries;
    item.enqueueAt = std::chrono::steady_clock::now();
    pendingRequests_.push_back(item);
    MODBUS_TOOLS_VERBOSE_INFO("ModbusClient: enqueue request id={}, fc={}, slave={}, queue={}",
                              item.requestId,
                              static_cast<int>(item.functionCode),
                              item.slaveId,
                              pendingRequests_.size());
    return item.requestId;
}

void RequestExecutor::finishPendingRequest(int requestId, bool success,
                                           const QString& error) {
    std::lock_guard<std::mutex> pendingLock(pendingMutex_);
    auto it = std::find_if(pendingRequests_.begin(), pendingRequests_.end(),
                           [requestId](const PendingRequest& item) {
                               return item.requestId == requestId;
                           });
    if (it == pendingRequests_.end()) {
        spdlog::warn("ModbusClient: request id={} not found in queue", requestId);
        return;
    }
    const auto waitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - it->enqueueAt).count();
    MODBUS_TOOLS_VERBOSE_INFO(
        "ModbusClient: finish request id={}, success={}, queue_wait={}ms, error='{}'",
        requestId, success, waitMs, error.toStdString());
    pendingRequests_.erase(it);
}

} // namespace modbus::session