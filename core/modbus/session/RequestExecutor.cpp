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
#include <tuple>
#include <QtEndian>
#include "infra/logging/TraceContext.h"
#include <QtGlobal>
#include <QEventLoop>
#include <QThread>

namespace modbus::session {
namespace {

    void warnIfTraceLost() {
        static std::once_flag flag;
        std::call_once(flag, [] {
            SPDLOG_ERROR("RequestExecutor: trace context lost (trace_id=0 on a "
                          "path that should carry a trace). Check ModbusWorker "
                          "thread affinity.");
        });
    }

    unsigned long long currentTrace() {
        return static_cast<unsigned long long>(modbus::trace::currentTraceId);
    }

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

    enum class FailureKind : uint8_t {
        Timeout = 0,
        RtuFrameTimeout = 1,
        FullPacketTimeout = 2,
        Retry = 3,
    };

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

RequestExecutor::RequestExecutor(const Dependencies& deps)
    : channel_(deps.channel)
    , transport_(deps.transport)
    , frameExtractor_(deps.frameExtractor)
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
    Q_ASSERT(retryStrategy_);
    Q_ASSERT(connStateMachine_);
    Q_ASSERT(reqStateMachine_);
    Q_ASSERT(connectionManager_);
    Q_ASSERT(config_);
}

RequestExecutor::~RequestExecutor() {
    if (transactionTimer_) {
        transactionTimer_->stop();
    }
    if (rtuSilenceTimer_) {
        rtuSilenceTimer_->stop();
    }
}

// --- Public API ---

bool RequestExecutor::tryAcquireRequestLock() {
    if (requestLocked_) return false;
    requestLocked_ = true;
    return true;
}

common::Result<base::Pdu, base::ModbusError> RequestExecutor::executeRequest(const base::Pdu& request, int slaveId) {
    return execute(request, slaveId).toResult();
}

ModbusResponse RequestExecutor::execute(const base::Pdu& request, int slaveId) {
    ModbusResponse finalResponse = ModbusResponse::Error(base::ModbusError::Unknown);
    bool finished = false;
    QEventLoop loop;
    executeAsync(request, slaveId, 0, [&finalResponse, &finished, &loop](const ModbusResponse& res) {
        finalResponse = res;
        finished = true;
        if (loop.isRunning()) {
            loop.quit();
        }
    });
    if (!finished) {
        loop.exec();
    }
    return finalResponse;
}

void RequestExecutor::executeAsync(const base::Pdu& request, int slaveId, quint64 traceId,
                                   AsyncResponseCallback callback) {
    if (!tryAcquireRequestLock()) {
        SPDLOG_WARN("RequestExecutor: rejected concurrent sendRequest while another request is active trace_id={}",
                    currentTrace());
        if (callback) {
            callback(ModbusResponse::Busy());
        }
        return;
    }

    if (aborted_.load(std::memory_order_acquire)) {
        requestLocked_ = false;
        reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted, "request-aborted-before-lock");
        if (callback) {
            callback(ModbusResponse::Error(base::ModbusError::Aborted,
                                         QStringLiteral("Aborted"),
                                         retryStrategy_->attemptCount()));
        }
        return;
    }
    aborted_ = false;

    if (!connectionManager_->ensureConnected()) {
        requestLocked_ = false;
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "not-connected");
        const QString channelError = connectionManager_->lastChannelError();
        if (callback) {
            callback(ModbusResponse::Error(base::ModbusError::NotConnected,
                                         channelError.isEmpty() ? QStringLiteral("Not connected") : channelError));
        }
        return;
    }

    const int targetSlaveId = (slaveId == -1) ? config_->slaveId : slaveId;
    const auto validationResult = request_validator::validate(request, targetSlaveId, config_->mode);
    if (!validationResult.valid) {
        requestLocked_ = false;
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "request-validation-failed");
        if (callback) {
            callback(ModbusResponse::Error(base::ModbusError::InvalidPdu, validationResult.error));
        }
        return;
    }

    retryStrategy_->reset();
    activeRequest_ = request;
    activeSlaveId_ = targetSlaveId;
    activeTraceId_ = traceId;
    activeCallback_ = std::move(callback);
    activeRequestId_ = enqueuePendingRequest(request, targetSlaveId);
    reqStateMachine_->tryTransition(RequestStateMachine::State::Idle, "request-start");

    if (!transactionTimer_) {
        transactionTimer_ = std::make_unique<QTimer>();
        transactionTimer_->setSingleShot(true);
        transactionTimer_->setTimerType(Qt::PreciseTimer);
        QObject::connect(transactionTimer_.get(), &QTimer::timeout, [this]() {
            handleTimeout();
        });
    }
    if (!rtuSilenceTimer_) {
        rtuSilenceTimer_ = std::make_unique<QTimer>();
        rtuSilenceTimer_->setSingleShot(true);
        rtuSilenceTimer_->setTimerType(Qt::PreciseTimer);
        QObject::connect(rtuSilenceTimer_.get(), &QTimer::timeout, [this]() {
            checkPendingResponse();
        });
    }

    sendCurrentRequest();
}

void RequestExecutor::sendCurrentRequest() {
    if (aborted_.load()) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted, "request-aborted-before-send");
        completeTransaction(ModbusResponse::Error(base::ModbusError::Aborted, QStringLiteral("Aborted")));
        return;
    }

    frameExtractor_->reset();
    responseReady_ = false;
    if (config_->mode == base::ModbusMode::RTU) {
        writeDrained_ = false;
    }
    transport_->resetPendingState();

    if (isBroadcastRequest(activeSlaveId_, activeRequest_.functionCode())
        && !isBroadcastWriteFunction(activeRequest_.functionCode())) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "invalid-broadcast-function");
        completeTransaction(ModbusResponse::Error(base::ModbusError::InvalidPdu,
            QStringLiteral("Broadcast only supports write function codes")));
        return;
    }

    const QByteArray adu = transport_->buildRequest(activeRequest_, activeSlaveId_);

    const auto now = std::chrono::steady_clock::now();
    if (config_->mode == base::ModbusMode::RTU && now < rtuSendWindowOpensAt_) {
        const auto waitDelay = std::chrono::duration_cast<std::chrono::milliseconds>(rtuSendWindowOpensAt_ - now);
        QTimer::singleShot(std::max(1, static_cast<int>(waitDelay.count())), [this, adu]() {
            performWrite(adu);
        });
    } else {
        performWrite(adu);
    }
}

void RequestExecutor::performWrite(const QByteArray& adu) {
    if (aborted_.load()) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted, "request-aborted-before-write");
        completeTransaction(ModbusResponse::Error(base::ModbusError::Aborted, QStringLiteral("Aborted")));
        return;
    }

    if (!channel_->write(adu)) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "write-failed");
        const QString error = connectionManager_->hasChannelError()
            ? connectionManager_->lastChannelError()
            : QString::fromLatin1(base::toString(base::ModbusError::WriteFailed));
        completeTransaction(ModbusResponse::Error(base::ModbusError::WriteFailed, error));
        return;
    }

    // Update RTU quiet send window
    if (config_->mode == base::ModbusMode::RTU) {
        const int bitsPerChar = 11;
        const int charTimeUs = (config_->baudRate > 0) ? (bitsPerChar * 1000000 / config_->baudRate) : 1000;
        const auto frameTxDuration = std::chrono::microseconds(charTimeUs * adu.size());
        const auto silence = FrameExtractor::calculateInterFrameDelay(*config_);
        rtuSendWindowOpensAt_ = std::chrono::steady_clock::now() + frameTxDuration + silence;
    }

    reqStateMachine_->tryTransition(RequestStateMachine::State::Sending, "write-success");
    transactionStartTime_ = std::chrono::steady_clock::now();

    if (!shouldWaitForResponse(activeSlaveId_, activeRequest_.functionCode())) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Completed, "broadcast-write-no-response");
        completeTransaction(ModbusResponse::NoResponseExpected(
            base::Pdu(activeRequest_.functionCode(), activeRequest_.data())));
        return;
    }

    reqStateMachine_->tryTransition(RequestStateMachine::State::Waiting, "wait-response");

    if (!transactionTimer_) {
        transactionTimer_ = std::make_unique<QTimer>();
        transactionTimer_->setSingleShot(true);
        transactionTimer_->setTimerType(Qt::PreciseTimer);
        QObject::connect(transactionTimer_.get(), &QTimer::timeout, [this]() {
            handleTimeout();
        });
    }

    int timeoutMs = config_->timeoutMs;
    if (config_->mode == base::ModbusMode::RTU) {
        timeoutMs += static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                FrameExtractor::calculateInterFrameDelay(*config_)).count());
    }
    transactionTimer_->start(std::max(10, timeoutMs));
}

void RequestExecutor::sendRaw(const QByteArray& data) {
    if (!tryAcquireRequestLock()) {
        SPDLOG_DEBUG("RequestExecutor: rejected sendRaw while another request is active trace_id={}",
                     currentTrace());
        return;
    }
    RequestLockGuard unlockGuard(requestLocked_);
    channel_->write(data);
}

void RequestExecutor::abort() {
    SPDLOG_INFO("ModbusClient: Abort requested trace_id={}", currentTrace());
    aborted_ = true;
    if (transactionTimer_) {
        transactionTimer_->stop();
    }
    if (rtuSilenceTimer_) {
        rtuSilenceTimer_->stop();
    }
    const auto current = reqStateMachine_->currentState();
    if (current != RequestStateMachine::State::Completed && current != RequestStateMachine::State::Failed) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted, "abort");
    }
    if (activeCallback_) {
        completeTransaction(ModbusResponse::Error(base::ModbusError::Aborted,
                                                 QStringLiteral("Aborted"),
                                                 retryStrategy_->attemptCount()));
    }
}

// --- Callbacks ---

void RequestExecutor::onDataReceived(QByteArrayView data) {
    SPDLOG_DEBUG("ModbusClient: Data received, size={}, trace_id={}", data.size(), currentTrace());
    frameExtractor_->feed(data);

    if (!activeCallback_) {
        return;
    }

    if (transactionTimer_ && QThread::currentThread() != transactionTimer_->thread()) {
        QMetaObject::invokeMethod(transactionTimer_.get(), [this]() {
            checkPendingResponse();
        }, Qt::QueuedConnection);
    } else {
        checkPendingResponse();
    }
}

void RequestExecutor::checkPendingResponse() {
    if (!activeCallback_) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    std::optional<QByteArray> frameOpt;
    if (config_->mode == base::ModbusMode::RTU || config_->mode == base::ModbusMode::ASCII) {
        if (frameExtractor_->isRtuFrameReadyToParse(now)) {
            frameOpt = frameExtractor_->tryPopRtuResponseFrame(now);
        } else if (frameExtractor_->bufferSize() > 0) {
            const auto boundary = frameExtractor_->nextRtuFrameBoundary();
            if (boundary > now) {
                const auto waitUs = std::chrono::duration_cast<std::chrono::microseconds>(boundary - now).count();
                const int waitMs = std::max(1, static_cast<int>((waitUs + 999) / 1000));
                if (rtuSilenceTimer_) {
                    rtuSilenceTimer_->start(waitMs);
                }
            }
        }
    } else {
        frameOpt = frameExtractor_->popFrame();
    }

    if (frameOpt) {
        if (rtuSilenceTimer_) {
            rtuSilenceTimer_->stop();
        }
        auto parsedResult = handleParsedFrame(*frameOpt, activeRequest_, activeSlaveId_, transactionStartTime_);
        if (parsedResult.has_value()) {
            if (parsedResult->isError()) {
                retryStrategy_->recordAttempt();
                if (retryStrategy_->shouldRetry() && !aborted_.load()) {
                    reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "parse-failed-retry");
                    const auto retryDelay = retryStrategy_->nextWait();
                    const auto retryKey = std::make_tuple(
                        static_cast<uint8_t>(activeSlaveId_),
                        static_cast<uint8_t>(activeRequest_.functionCode()),
                        static_cast<uint8_t>(FailureKind::Retry));
                    if (failureDedupe_.shouldLog(retryKey, std::chrono::steady_clock::now())) {
                        SPDLOG_WARN("Request failed, retrying... ({}/{}) trace_id={}",
                                     retryStrategy_->attemptCount(), config_->retries, currentTraceRequired());
                    }
                    QTimer::singleShot(std::max<int>(1, static_cast<int>(retryDelay.count())), [this]() {
                        sendCurrentRequest();
                    });
                    return;
                }
            }
            completeTransaction(*parsedResult);
            return;
        }
    }

    if (frameExtractor_->hasExceededDropLimit()) {
        if (rtuSilenceTimer_) {
            rtuSilenceTimer_->stop();
        }
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "too-many-invalid-bytes");
        completeTransaction(ModbusResponse::Error(base::ModbusError::TooManyInvalidBytes,
            QStringLiteral("Too many invalid response bytes")));
    }
}

void RequestExecutor::onChannelError(const QString& error) {
    connectionManager_->setError(error);
    if (!activeCallback_) {
        return;
    }
    if (transactionTimer_ && QThread::currentThread() != transactionTimer_->thread()) {
        QMetaObject::invokeMethod(transactionTimer_.get(), [this, error]() {
            if (activeCallback_) {
                completeTransaction(ModbusResponse::Error(base::ModbusError::ConnectionFailed, error));
            }
        }, Qt::QueuedConnection);
    } else {
        completeTransaction(ModbusResponse::Error(base::ModbusError::ConnectionFailed, error));
    }
}

void RequestExecutor::onWriteDrained() {
    writeDrained_ = true;
    lastWriteDrainedAt_ = std::chrono::steady_clock::now();
}

void RequestExecutor::resetState(bool clearPendingQueue) {
    responseReady_ = false;
    writeDrained_ = true;
    lastWriteDrainedAt_ = {};
    rtuSendWindowOpensAt_ = {};
    if (transactionTimer_) {
        transactionTimer_->stop();
    }
    if (rtuSilenceTimer_) {
        rtuSilenceTimer_->stop();
    }
    if (clearPendingQueue) {
        pendingRequests_.clear();
    }
    if (activeCallback_) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "reset-state");
        completeTransaction(ModbusResponse::Error(base::ModbusError::ConnectionFailed,
                                                 QStringLiteral("Disconnected during active request")));
    }
}

void RequestExecutor::handleTimeout() {
    reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "timeout");
    connectionManager_->onRequestTimeout();

    const auto key = std::make_tuple(
        static_cast<uint8_t>(activeSlaveId_),
        static_cast<uint8_t>(activeRequest_.functionCode()),
        static_cast<uint8_t>(FailureKind::Timeout));
    if (failureDedupe_.shouldLog(key, std::chrono::steady_clock::now())) {
        SPDLOG_WARN("ModbusClient: request timeout slave={} fc={} timeoutMs={} trace_id={}",
                     activeSlaveId_, static_cast<int>(activeRequest_.functionCode()),
                     config_->timeoutMs, currentTraceRequired());
    } else {
        SPDLOG_DEBUG("ModbusClient: request timeout slave={} fc={} timeoutMs={} trace_id={} (duplicate within {}s)",
                     activeSlaveId_, static_cast<int>(activeRequest_.functionCode()),
                     config_->timeoutMs, currentTraceRequired(),
                     kDupeTrackerSuppressionWindowSeconds);
    }

    retryStrategy_->recordAttempt();
    if (retryStrategy_->shouldRetry() && !aborted_.load()) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "request-retry");
        const auto retryDelay = retryStrategy_->nextWait();
        const auto retryKey = std::make_tuple(
            static_cast<uint8_t>(activeSlaveId_),
            static_cast<uint8_t>(activeRequest_.functionCode()),
            static_cast<uint8_t>(FailureKind::Retry));
        if (failureDedupe_.shouldLog(retryKey, std::chrono::steady_clock::now())) {
            SPDLOG_WARN("Request failed, retrying... ({}/{}) trace_id={}",
                         retryStrategy_->attemptCount(), config_->retries, currentTraceRequired());
        }
        QTimer::singleShot(std::max<int>(1, static_cast<int>(retryDelay.count())), [this]() {
            sendCurrentRequest();
        });
    } else {
        completeTransaction(ModbusResponse::Error(base::ModbusError::Timeout, QStringLiteral("Timeout")));
    }
}

void RequestExecutor::completeTransaction(ModbusResponse response) {
    if (transactionTimer_) {
        transactionTimer_->stop();
    }
    if (rtuSilenceTimer_) {
        rtuSilenceTimer_->stop();
    }
    if (response.attemptCount <= 0) {
        response.attemptCount = response.isError()
            ? std::max(1, retryStrategy_->attemptCount())
            : (retryStrategy_->attemptCount() + 1);
    }
    if (response.rttMs < 0 && transactionStartTime_ != std::chrono::steady_clock::time_point{}) {
        response.rttMs = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - transactionStartTime_).count());
    }

    if (!response.isError()) {
        reqStateMachine_->tryTransition(RequestStateMachine::State::Completed, "request-success");
        finishPendingRequest(activeRequestId_, true, QString());
    } else {
        if (aborted_.load()) {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Aborted, "request-aborted");
        } else {
            reqStateMachine_->tryTransition(RequestStateMachine::State::Failed, "request-failed");
        }
        finishPendingRequest(activeRequestId_, false, response.error);
    }

    requestLocked_ = false;
    auto cb = std::move(activeCallback_);
    activeCallback_ = nullptr;
    if (cb) {
        cb(response);
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
            return std::nullopt;
        }
        const QString responseValidationError = base::validateResponsePdu(request, pdu);
        if (!responseValidationError.isEmpty()) {
            reqStateMachine_->tryTransition(
                RequestStateMachine::State::Failed, "response-validation-failed");
            return ModbusResponse::Error(base::ModbusError::InvalidPdu, responseValidationError);
        }
        auto rttMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        connectionManager_->onRequestSuccess();
        reqStateMachine_->tryTransition(
            RequestStateMachine::State::Completed, "response-parsed");
        return ModbusResponse::Success(pdu, static_cast<int>(rttMs));
    }
    if (parseResult.status == transport::ParseResponseStatus::Unmatched) {
        return std::nullopt;
    }
    reqStateMachine_->tryTransition(
        RequestStateMachine::State::Failed, "response-parse-failed");
    return ModbusResponse::Error(base::ModbusError::InvalidAdu,
        QStringLiteral("Response parsing failed"));
}

ModbusResponse RequestExecutor::handleExceptionResponse(const base::Pdu& responsePdu, int slaveId,
                                                        const base::Pdu& requestPdu) {
    const unsigned long long traceId = currentTrace();
    auto dupeKey = std::make_tuple(static_cast<uint8_t>(slaveId),
                                   static_cast<uint8_t>(requestPdu.functionCode()),
                                   static_cast<uint8_t>(responsePdu.exceptionCode()));
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
    return ModbusResponse::Error(base::ModbusError::ExceptionResponse,
                                 QString(),
                                 0,
                                 responsePdu.exceptionCode());
}

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
