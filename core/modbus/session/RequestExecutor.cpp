/**
 * @file RequestExecutor.cpp
 * @brief Implementation of RequestExecutor.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "RequestExecutor.h"
#include "RequestValidator.h"
#include "Config.h"
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
#include "common/TrContext.h"
#include "infra/logging/TraceContext.h"
#include <QtGlobal>

namespace modbus::session {
namespace {

    // One-shot guard: if a log site that should carry a trace id observes 0,
    // the worker thread affinity was broken somewhere upstream. Warn once so
    // the regression is visible without flooding the log on every request.
    void warnIfTraceLost() {
        static std::once_flag flag;
        std::call_once(flag, [] {
            SPDLOG_ERROR("RequestExecutor: trace context lost (trace_id=0 on a "
                          "path that should carry a trace). Check ModbusWorker "
                          "thread affinity.");
        });
    }

    // Read the thread-local trace id published by ModbusWorker::handleSubmit.
    // Returns 0 when no request context is active (e.g. channel callbacks
    // fired while idle) — callers decide whether 0 is expected.
    unsigned long long currentTrace() {
        return static_cast<unsigned long long>(modbus::trace::currentTraceId);
    }

    // Variant for request-path log sites (retry / timeouts) where a trace id
    // must be present; emits a one-shot error if the context was lost.
    unsigned long long currentTraceRequired() {
        const unsigned long long id = currentTrace();
        if (id == 0) {
            warnIfTraceLost();
        }
        return id;
    }

    constexpr std::size_t kDupeTrackerCleanupThreshold = 1000;
    constexpr auto kDupeTrackerSuppressionWindow = std::chrono::seconds(5);
    constexpr auto kDupeTrackerSuppressionWindowSeconds =
        std::chrono::duration_cast<std::chrono::seconds>(
            kDupeTrackerSuppressionWindow).count();

    // Failure kinds for the timeout/retry deduplication key
    // (slave, fc, errorKind). Integer enum by design — see LogDedupe.h.
    enum class FailureKind : uint8_t {
        Timeout = 0,
        RtuFrameTimeout = 1,
        FullPacketTimeout = 2,
        Retry = 3,
    };

    constexpr char kReqExecCtx[] = "modbus::session::RequestExecutor";

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

    QString exceptionName(base::ExceptionCode code) {
        using base::ExceptionCode;
        switch (code) {
        case ExceptionCode::IllegalFunction:
            return TrContext<kReqExecCtx>::tr("Illegal Function");
        case ExceptionCode::IllegalDataAddress:
            return TrContext<kReqExecCtx>::tr("Illegal Data Address");
        case ExceptionCode::IllegalDataValue:
            return TrContext<kReqExecCtx>::tr("Illegal Data Value");
        case ExceptionCode::ServerDeviceFailure:
            return TrContext<kReqExecCtx>::tr("Server Device Failure");
        case ExceptionCode::Acknowledge:
            return TrContext<kReqExecCtx>::tr("Acknowledge");
        case ExceptionCode::ServerDeviceBusy:
            return TrContext<kReqExecCtx>::tr("Server Device Busy");
        case ExceptionCode::NegativeAcknowledge:
            return TrContext<kReqExecCtx>::tr("Negative Acknowledge");
        case ExceptionCode::MemoryParityError:
            return TrContext<kReqExecCtx>::tr("Memory Parity Error");
        case ExceptionCode::GatewayPathUnavailable:
            return TrContext<kReqExecCtx>::tr("Gateway Path Unavailable");
        case ExceptionCode::GatewayTargetDeviceFailed:
            return TrContext<kReqExecCtx>::tr("Gateway Target Device Failed To Respond");
        default:
            return TrContext<kReqExecCtx>::tr("Unknown Exception");
        }
    }

    QString buildExceptionMessage(int slaveId,
                                  base::FunctionCode requestFc,
                                  base::ExceptionCode exceptionCode) {
        return TrContext<kReqExecCtx>::tr("Modbus exception response. Slave=%1 FC=0x%2 Exception=0x%3 (%4)")
            .arg(slaveId)
            .arg(static_cast<int>(requestFc), 2, 16, QChar('0'))
            .arg(static_cast<int>(exceptionCode), 2, 16, QChar('0'))
            .arg(exceptionName(exceptionCode));
    }

} // namespace

RequestExecutor::RequestExecutor(const Dependencies& deps)
    : channel_(deps.channel)
    , transport_(deps.transport)
    , frameExtractor_(deps.frameExtractor)
    , flowController_(deps.flowController)
    , retryStrategy_(deps.retryStrategy)
    , connStateMachine_(deps.connStateMachine)
    , reqStateMachine_(deps.reqStateMachine)
    , connectionManager_(deps.connectionManager)
    , config_(deps.config)
    , aborted_(deps.aborted)
    , exceptionDedupe_(kDupeTrackerSuppressionWindow)
    , failureDedupe_(kDupeTrackerSuppressionWindow) {
    Q_ASSERT(channel_);
    Q_ASSERT(transport_);
    Q_ASSERT(frameExtractor_);
    Q_ASSERT(flowController_);
    Q_ASSERT(retryStrategy_);
    Q_ASSERT(connStateMachine_);
    Q_ASSERT(reqStateMachine_);
    Q_ASSERT(connectionManager_);
    Q_ASSERT(config_);
}

// --- Public API ---

bool RequestExecutor::tryAcquireRequestLock() {
    if (requestLocked_) return false;
    requestLocked_ = true;
    return true;
}

ModbusResponse RequestExecutor::execute(const base::Pdu& request, int slaveId) {
    if (!tryAcquireRequestLock()) {
        // Busy, not an error: while a request blocks in waitForCondition()
        // (which still pumps the worker event loop), re-entrant submits are
        // an expected artifact of that wait — answer them with the
        // structured Busy response instead of a warn-level rejection log,
        // so a connect-wait storm does not flood production logs.
        SPDLOG_DEBUG("RequestExecutor: rejected concurrent sendRequest while another request is active trace_id={}",
                     currentTrace());
        return ModbusResponse::Busy(TrContext<kReqExecCtx>::tr("Request already in progress"));
    }
    RequestLockGuard unlockGuard(requestLocked_);
    if (aborted_.load(std::memory_order_acquire)) {
        // Abort was requested before we acquired the lock; don't reset it.
        reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                        "request-aborted-before-lock");
        return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Aborted"), retryStrategy_->attemptCount());
    }
    aborted_ = false;

    retryStrategy_->reset();
    ModbusResponse lastResponse = ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Unknown error"));
    const int requestId = enqueuePendingRequest(request, slaveId);
    reqStateMachine_->tryTransition(RequestStateMachine::State::Idle, "request-start");

    for (;;) {
        if (aborted_) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                            "request-aborted-before-send");
            finishPendingRequest(requestId, false, "Aborted");
            return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Aborted"), retryStrategy_->attemptCount());
        }

        lastResponse = sendRequestInternal(request, slaveId);
        lastResponse.attemptCount = retryStrategy_->attemptCount() + 1;
        if (!lastResponse.isError()) {
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
            // Dedupe: under polling of a dead device this warn would fire on
            // every request. First occurrence per (slave, fc) within the
            // window logs at warn; duplicates drop to debug.
            const auto retryKey = std::make_tuple(
                static_cast<uint8_t>(slaveId),
                static_cast<uint8_t>(request.functionCode()),
                static_cast<uint8_t>(FailureKind::Retry));
            if (failureDedupe_.shouldLog(retryKey, std::chrono::steady_clock::now())) {
                SPDLOG_WARN("Request failed, retrying... ({}/{}) trace_id={} Error: {}",
                             retryStrategy_->attemptCount(),
                             config_->retries,
                             currentTraceRequired(),
                             lastResponse.error.toStdString());
            } else {
                SPDLOG_DEBUG("Request failed, retrying... ({}/{}) trace_id={} Error: {} (duplicate within {}s)",
                              retryStrategy_->attemptCount(),
                              config_->retries,
                              currentTraceRequired(),
                              lastResponse.error.toStdString(),
                              kDupeTrackerSuppressionWindowSeconds);
            }
            if (!waitForAbortableDelay(aborted_, retryDelay)) {
                reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                                "request-aborted-during-backoff");
                finishPendingRequest(requestId, false, "Aborted");
                return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Aborted"), retryStrategy_->attemptCount());
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
    if (!tryAcquireRequestLock()) {
        // Same busy-mitigation rationale as execute(): see the comment there.
        SPDLOG_DEBUG("RequestExecutor: rejected sendRaw while another request is active trace_id={}",
                     currentTrace());
        return;
    }
    RequestLockGuard unlockGuard(requestLocked_);
    if (connectionManager_->isConnected()) {
        if (!flowController_->isRtuSendWindowOpen(std::chrono::steady_clock::now())) {
            waitForAbortableDelay(aborted_,
                flowController_->rtuSendWindowOpensAt() - std::chrono::steady_clock::now());
        }
        if (config_->mode == base::ModbusMode::RTU) {
            flowController_->markWritePending();
        }
        writeRtuFrameWithDrain(data, nullptr);
    }
}

void RequestExecutor::abort() {
    // abort() may be called from any thread (e.g. the UI thread during stop),
    // so trace_id can legitimately be 0 here.
    SPDLOG_INFO("ModbusClient: Abort requested trace_id={}", currentTrace());
    aborted_ = true;
    const auto current = reqStateMachine_->currentState();
    if (current != RequestStateMachine::State::Completed
        && current != RequestStateMachine::State::Failed) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted, "abort");
    }
}

// --- Callbacks ---

void RequestExecutor::onDataReceived(QByteArrayView data) {
    // Channel callback: trace_id is 0 when data arrives outside a request.
    SPDLOG_DEBUG("ModbusClient: Data received, size={}, notifying loop trace_id={}",
                  data.size(), currentTrace());

    frameExtractor_->feed(data);
    responseReady_ = frameExtractor_->hasCompleteFrame()
        || (config_->mode == base::ModbusMode::RTU && frameExtractor_->bufferSize() > 0);
}

void RequestExecutor::onChannelError(const QString& error) {
    connectionManager_->setError(error);
    // trace_id is 0 when the error arrives outside a request context
    // (channel callbacks run on the channel thread); non-zero when a request
    // is in flight on the worker thread.
    SPDLOG_WARN("ModbusClient: channel error forwarded: '{}' state={} connState={} trace_id={}",
                 error.toStdString(),
                 static_cast<int>(channel_->state()),
                 static_cast<int>(connStateMachine_->currentState()),
                 currentTrace());
}

void RequestExecutor::resetState(bool clearPendingQueue) {
    responseReady_ = false;
    exceptionDedupe_.clear();
    failureDedupe_.clear();
    if (clearPendingQueue) {
        pendingRequests_.clear();
    }
}

// --- Private: Request flow ---

ModbusResponse RequestExecutor::sendRequestInternal(const base::Pdu& request, int slaveId) {
    if (!connectionManager_->ensureConnected(config_->autoReconnect)) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "not-connected");
        const QString channelError = connectionManager_->lastChannelError();
        return ModbusResponse::Error(channelError.isEmpty()
                                         ? TrContext<kReqExecCtx>::tr("Not connected")
                                         : channelError);
    }

    const int targetSlaveId = (slaveId == -1) ? config_->slaveId : slaveId;
    const auto validationResult = request_validator::validate(request, targetSlaveId, config_->mode);
    if (!validationResult.valid) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                        "request-validation-failed");
        return ModbusResponse::Error(validationResult.error);
    }

    // 1. Cleanup old buffers and state
    if (aborted_) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                        "aborted-before-build");
        return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Aborted"));
    }
    frameExtractor_->reset();
    responseReady_ = false;
    if (config_->mode == base::ModbusMode::RTU) {
        flowController_->markWritePending();
    }
    transport_->resetPendingState();

    // 2. Build ADU
    if (isBroadcastRequest(targetSlaveId, request.functionCode())
        && !isBroadcastWriteFunction(request.functionCode())) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                        "invalid-broadcast-function");
        return ModbusResponse::Error(
            TrContext<kReqExecCtx>::tr("Broadcast only supports write function codes"));
    }
    QByteArray adu = transport_->buildRequest(request, targetSlaveId);

    if (!flowController_->isRtuSendWindowOpen(std::chrono::steady_clock::now())) {
        waitForAbortableDelay(aborted_,
            flowController_->rtuSendWindowOpensAt() - std::chrono::steady_clock::now());
    }

    std::chrono::steady_clock::time_point drainedAt{};
    if (!writeRtuFrameWithDrain(adu, &drainedAt)) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                        config_->mode == base::ModbusMode::RTU
                                            ? "write-drain-timeout"
                                            : "write-failed");
        const QString error = connectionManager_->hasChannelError()
            ? connectionManager_->lastChannelError()
            : TrContext<kReqExecCtx>::tr(config_->mode == base::ModbusMode::RTU
                        ? "Write drain timeout"
                        : "Write failed");
        return ModbusResponse::Error(error);
    }
    reqStateMachine_->tryTransition(RequestStateMachine::State::Sending, "write-success");
    auto start = std::chrono::steady_clock::now();
    if (drainedAt != std::chrono::steady_clock::time_point{}) {
        start = drainedAt;
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
    SPDLOG_DEBUG("ModbusClient: Entering wait loop, deadline in {}ms trace_id={}",
                              config_->timeoutMs, currentTrace());

    while (true) {
        const auto now = std::chrono::steady_clock::now();
        const bool serialFrameReady = frameExtractor_->isRtuFrameReadyToParse(now);
        if (!serialFrameReady && !responseReady_
            && !connectionManager_->hasChannelError() && !aborted_.load()) {
            const bool stillWaiting = waitForEventOrTimeout(deadline);
            if (!stillWaiting && std::chrono::steady_clock::now() >= deadline) {
                reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "timeout");
                // Half-open detection feed (T2.2): a genuine response-wait
                // timeout of an established session. Early exits of
                // waitForEventOrTimeout() (abort / data / channel error) do
                // not reach this site, so only real timeouts count toward
                // the consecutive-timeout eviction.
                connectionManager_->onRequestTimeout();
                // Dedupe: polling a dead device fires this on every request.
                const auto key = std::make_tuple(
                    static_cast<uint8_t>(slaveId),
                    static_cast<uint8_t>(request.functionCode()),
                    static_cast<uint8_t>(FailureKind::Timeout));
                if (failureDedupe_.shouldLog(key, std::chrono::steady_clock::now())) {
                    SPDLOG_WARN("ModbusClient: request timeout slave={} fc={} timeoutMs={} trace_id={}",
                                 slaveId, static_cast<int>(request.functionCode()),
                                 config_->timeoutMs, currentTraceRequired());
                } else {
                    SPDLOG_DEBUG("ModbusClient: request timeout slave={} fc={} timeoutMs={} trace_id={} (duplicate within {}s)",
                                  slaveId, static_cast<int>(request.functionCode()),
                                  config_->timeoutMs, currentTraceRequired(),
                                  kDupeTrackerSuppressionWindowSeconds);
                }
                return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Timeout"));
            }
            continue;
        }
        if (!serialFrameReady && config_->mode == base::ModbusMode::RTU
            && frameExtractor_->bufferSize() > 0) {
            responseReady_ = false;
            const auto frameDeadline = std::min(deadline,
                                                frameExtractor_->nextRtuFrameBoundary());
            const bool stillWaiting = waitForEventOrTimeout(frameDeadline);
            if (!stillWaiting && std::chrono::steady_clock::now() >= deadline) {
                reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "timeout");
                // Half-open detection feed (T2.2), see the main timeout site.
                connectionManager_->onRequestTimeout();
                const auto key = std::make_tuple(
                    static_cast<uint8_t>(slaveId),
                    static_cast<uint8_t>(request.functionCode()),
                    static_cast<uint8_t>(FailureKind::RtuFrameTimeout));
                if (failureDedupe_.shouldLog(key, std::chrono::steady_clock::now())) {
                    SPDLOG_WARN("ModbusClient: RTU frame wait timeout slave={} fc={} timeoutMs={} trace_id={}",
                                 slaveId, static_cast<int>(request.functionCode()),
                                 config_->timeoutMs, currentTraceRequired());
                } else {
                    SPDLOG_DEBUG("ModbusClient: RTU frame wait timeout slave={} fc={} timeoutMs={} trace_id={} (duplicate within {}s)",
                                  slaveId, static_cast<int>(request.functionCode()),
                                  config_->timeoutMs, currentTraceRequired(),
                                  kDupeTrackerSuppressionWindowSeconds);
                }
                return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Timeout"));
            }
            continue;
        }
        if (aborted_) {
            SPDLOG_DEBUG("ModbusClient: Aborted during wait trace_id={}", currentTrace());
            reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted,
                                            "aborted-during-wait");
            return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Aborted"));
        }
        
        const QString chErr = connectionManager_->lastChannelError();
        if (!chErr.isEmpty()) {
                reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                                "channel-error");
                // Lifecycle closure: a channel error tearing down the session
                // mid-request is a *passive* disconnect — mark it explicitly
                // so log readers can distinguish it from user-initiated ones.
                SPDLOG_INFO("ModbusClient: session disconnected passively by channel error: '{}' trace_id={}",
                             chErr.toStdString(), currentTrace());
                return ModbusResponse::Error(chErr);
            }

        if (frameExtractor_->hasExceededDropLimit()) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                            "too-many-invalid-bytes");
            return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Too many invalid response bytes"));
        }

        responseReady_ = false;

        // Bound deduplication state without changing response behavior.
        exceptionDedupe_.prune(now, kDupeTrackerCleanupThreshold);
        failureDedupe_.prune(now, kDupeTrackerCleanupThreshold);

        while (true) {
            if (config_->mode == base::ModbusMode::RTU
                || config_->mode == base::ModbusMode::ASCII) {
                auto frameOpt = frameExtractor_->tryPopRtuResponseFrame(now);
                if (frameOpt) {
                    auto result = handleParsedFrame(*frameOpt, request, slaveId, start);
                    if (result) return *result;
                    continue;
                }
                break;
            }

            auto frameOpt = frameExtractor_->popFrame();
            if (frameOpt) {
                auto result = handleParsedFrame(*frameOpt, request, slaveId, start);
                if (result) return *result;
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
                    TrContext<kReqExecCtx>::tr("Incomplete RTU frame after inter-frame silence"));
            }
            break;
        }

        if (std::chrono::steady_clock::now() >= deadline) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Failed,
                                            "timeout-full-packet");
            // Half-open detection feed (T2.2), see the main timeout site.
            connectionManager_->onRequestTimeout();
            const auto key = std::make_tuple(
                static_cast<uint8_t>(slaveId),
                static_cast<uint8_t>(request.functionCode()),
                static_cast<uint8_t>(FailureKind::FullPacketTimeout));
            if (failureDedupe_.shouldLog(key, std::chrono::steady_clock::now())) {
                SPDLOG_WARN("ModbusClient: full packet wait timeout slave={} fc={} timeoutMs={} trace_id={}",
                             slaveId, static_cast<int>(request.functionCode()),
                             config_->timeoutMs, currentTraceRequired());
            } else {
                SPDLOG_DEBUG("ModbusClient: full packet wait timeout slave={} fc={} timeoutMs={} trace_id={} (duplicate within {}s)",
                              slaveId, static_cast<int>(request.functionCode()),
                              config_->timeoutMs, currentTraceRequired(),
                              kDupeTrackerSuppressionWindowSeconds);
            }
            return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Timeout while waiting for full packet"));
        }
    }
}

std::optional<ModbusResponse> RequestExecutor::handleParsedFrame(
    const QByteArray& frame,
    const base::Pdu& request,
    int slaveId,
    std::chrono::steady_clock::time_point start) {
    auto parseResult = transport_->parseResponse(frame);
    if (parseResult.status == transport::ParseResponseStatus::Ok && parseResult.pdu) {
        const auto& pdu = *parseResult.pdu;
        if (pdu.isException()) {
            reqStateMachine_->tryTransition(
                RequestStateMachine::State::Failed, "modbus-exception");
            return handleExceptionResponse(pdu, slaveId, request);
        }
        if (pdu.originalFunctionCode() != request.functionCode()) {
            return std::nullopt; // unmatched slave/function — keep waiting
        }
        const QString responseValidationError = base::validateResponsePdu(request, pdu);
        if (!responseValidationError.isEmpty()) {
            reqStateMachine_->tryTransition(
                RequestStateMachine::State::Failed, "response-validation-failed");
            return ModbusResponse::Error(responseValidationError);
        }
        auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        // Half-open detection feed (T2.2): a validated response proves the
        // session is live — reset the consecutive-timeout counter.
        connectionManager_->onRequestSuccess();
        reqStateMachine_->tryTransition(
            RequestStateMachine::State::Completed, "response-parsed");
        return ModbusResponse::Success(pdu, static_cast<int>(rttMs));
    }
    if (parseResult.status == transport::ParseResponseStatus::Unmatched) {
        return std::nullopt; // frame belongs to another transaction — keep waiting
    }
    // Invalid, or Ok without pdu (transport contract violation — treat as failure).
    reqStateMachine_->tryTransition(
        RequestStateMachine::State::Failed, "response-parse-failed");
    return ModbusResponse::Error(TrContext<kReqExecCtx>::tr("Response parsing failed"));
}

ModbusResponse RequestExecutor::handleExceptionResponse(const base::Pdu& responsePdu, int slaveId,
                                                        const base::Pdu& requestPdu) {
    // Read the trace id before taking the lock — a pure thread_local read,
    // keeps the critical section free of any trace-context access.
    const unsigned long long traceId = currentTrace();
    const QString exceptionMessage = buildExceptionMessage(
        slaveId,
        requestPdu.functionCode(),
        responsePdu.exceptionCode());
    auto dupeKey = std::make_tuple(static_cast<uint8_t>(slaveId),
                                   static_cast<uint8_t>(requestPdu.functionCode()),
                                   static_cast<uint8_t>(responsePdu.exceptionCode()));
    // shouldLog is a pure in-memory operation (map lookup + time compare),
    // safe to call while holding mutex_ — no string work happens inside it.
    if (!exceptionDedupe_.shouldLog(dupeKey, std::chrono::steady_clock::now())) {
        SPDLOG_DEBUG("ModbusClient: Modbus exception response. "
                      "Slave={} FC=0x{:02X} Exception=0x{:02X} (duplicate within {}s) trace_id={}",
                      slaveId, static_cast<int>(requestPdu.functionCode()),
                      static_cast<int>(responsePdu.exceptionCode()),
                      kDupeTrackerSuppressionWindowSeconds, traceId);
    } else {
        SPDLOG_DEBUG("ModbusClient: Modbus exception response. "
                      "Slave={} FC=0x{:02X} Exception=0x{:02X} trace_id={}",
                      slaveId, static_cast<int>(requestPdu.functionCode()),
                      static_cast<int>(responsePdu.exceptionCode()), traceId);
    }
    return ModbusResponse::Error(exceptionMessage);
}

// --- Private: Helpers ---

bool RequestExecutor::isBroadcastRequest(int slaveId,
                                          base::FunctionCode functionCode) const {
    Q_UNUSED(functionCode);
    return (config_->mode == base::ModbusMode::RTU
            || config_->mode == base::ModbusMode::ASCII)
        && slaveId == 0;
}

bool RequestExecutor::shouldWaitForResponse(int slaveId,
                                            base::FunctionCode functionCode) const {
    return !isBroadcastRequest(slaveId, functionCode);
}

bool RequestExecutor::writeRtuFrameWithDrain(const QByteArray& adu,
                                             std::chrono::steady_clock::time_point* drainedAt) {
    if (!channel_->write(adu)) {
        return false;
    }
    flowController_->updateRtuSendWindow(adu.size(), *config_);

    if (config_->mode != base::ModbusMode::RTU) {
        return true;
    }

    const auto writeDeadline = std::chrono::steady_clock::now()
        + std::chrono::milliseconds(config_->timeoutMs);
    return waitForWriteDrain(writeDeadline, drainedAt);
}

bool RequestExecutor::waitForWriteDrain(std::chrono::steady_clock::time_point deadline,
                                        std::chrono::steady_clock::time_point* drainedAt) {
    while (std::chrono::steady_clock::now() < deadline) {
        if (aborted_.load()) {
            return false;
        }
        if (flowController_->isWriteDrained()) {
            if (drainedAt) {
                const auto da = flowController_->drainedAt();
                *drainedAt = (da == std::chrono::steady_clock::time_point{})
                    ? std::chrono::steady_clock::now()
                    : da;
            }
            return true;
        }
        if (connectionManager_->hasChannelError()) {
            return false;
        }
        if (channel_->state() == io::ChannelState::Error) {
            return false;
        }

        waitForCondition([this]() {
            return aborted_.load() || flowController_->isWriteDrained()
                || connectionManager_->hasChannelError()
                || channel_->state() == io::ChannelState::Error;
        }, deadline);
    }

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
        if (aborted_.load() || responseReady_
            || connectionManager_->hasChannelError()
            || channel_->state() == io::ChannelState::Error) {
            return true;
        }

        waitForCondition([this]() {
            return aborted_.load() || responseReady_
                || connectionManager_->hasChannelError()
                || channel_->state() == io::ChannelState::Error;
        }, deadline);
    }
    return false;
}

int RequestExecutor::enqueuePendingRequest(const base::Pdu& request, int slaveId) {
    PendingRequest item;
    item.requestId = nextRequestId_++;
    item.functionCode = request.functionCode();
    item.slaveId = (slaveId == -1) ? config_->slaveId : slaveId;
    item.timeoutMs = config_->timeoutMs;
    item.retries = config_->retries;
    item.enqueueAt = std::chrono::steady_clock::now();
    pendingRequests_.push_back(item);
    SPDLOG_DEBUG("ModbusClient: enqueue request id={}, fc={}, slave={}, queue={} trace_id={}",
                              item.requestId,
                              static_cast<int>(item.functionCode),
                              item.slaveId,
                              pendingRequests_.size(),
                              currentTrace());
    return item.requestId;
}

void RequestExecutor::finishPendingRequest(int requestId, bool success,
                                           const QString& error) {
    auto it = std::find_if(pendingRequests_.begin(), pendingRequests_.end(),
                           [requestId](const PendingRequest& item) {
                               return item.requestId == requestId;
                           });
    if (it == pendingRequests_.end()) {
        SPDLOG_WARN("ModbusClient: request id={} not found in queue trace_id={}",
                     requestId, currentTrace());
        return;
    }
    const auto waitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - it->enqueueAt).count();
    SPDLOG_DEBUG(
        "ModbusClient: finish request id={}, success={}, queue_wait={}ms, error='{}' trace_id={}",
        requestId, success, waitMs, error.toStdString(), currentTrace());
    pendingRequests_.erase(it);
}

} // namespace modbus::session
