// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusPagePresenter.h"
#include "ModbusSessionPresenter.h"
#include "RequestSubmissionService.h"
#include "PollingController.h"
#include "TrafficLogController.h"
#include "modbus/session/SessionTypes.h"
#include "modbus/base/ModbusFunctionTraits.h"
#include <spdlog/spdlog.h>

namespace ui::application::modbus {

ModbusPagePresenter::ModbusPagePresenter(IModbusPageView* view,
                                         SessionMode mode,
                                         QObject* parent)
    : QObject(parent),
      view_(view),
      mode_(mode) {
    createServices();
    wireConnections();
}

ModbusPagePresenter::~ModbusPagePresenter() noexcept {
    teardownServices();
}

void ModbusPagePresenter::detachView() noexcept {
    view_ = nullptr;
}

void ModbusPagePresenter::createServices() {
    sessionPresenter_ = new ModbusSessionPresenter(mode_, this);
    requestService_ = std::make_unique<RequestSubmissionService>();
    pollingController_ = new PollingController(requestService_.get(), this);
    trafficLogController_ = new TrafficLogController(pollingController_, this);

    sessionPresenter_->setRequestService(requestService_.get());
    sessionPresenter_->setPollingController(pollingController_);
    sessionPresenter_->setTrafficLogController(trafficLogController_);

    connect(pollingController_, &PollingController::trafficEvent,
            trafficLogController_, &TrafficLogController::publishEvent);

    syncWidgetGuards(SessionConnectionState::Disconnected);
}

void ModbusPagePresenter::wireConnections() {
    requestService_->onTxCountUpdated = [this]() {
        recordTx();
    };

    connect(pollingController_, &PollingController::submitPollRequest,
            this, [this](const ::modbus::base::Pdu& pdu, int slaveId, int requestId, TraceId traceId) {
                if (sessionPresenter_) {
                    sessionPresenter_->submitRequest(pdu, slaveId, requestId, traceId);
                }
            });
    connect(pollingController_, &PollingController::summaryReady,
            trafficLogController_, &TrafficLogController::logPollSummary);
    connect(pollingController_, &PollingController::stopRequested,
            this, [this]() {
                syncWidgetGuards(sessionPresenter_ ? sessionPresenter_->connectionState()
                                                   : SessionConnectionState::Disconnected);
            });

    connect(trafficLogController_, &TrafficLogController::trafficEvent,
            this, [this](const ::ui::common::TrafficEvent& event) {
                if (view_) {
                    view_->appendTrafficTelemetry(event);
                }
            });

    if (sessionPresenter_) {
        connect(sessionPresenter_, &ModbusSessionPresenter::linkageSourceDisconnected,
                this, &ModbusPagePresenter::linkageSourceDisconnected);
        connect(sessionPresenter_, &ModbusSessionPresenter::requestFinished,
                this, &ModbusPagePresenter::handleRequestFinished);
        connect(sessionPresenter_, &ModbusSessionPresenter::connectFinished, this,
                [this](bool ok, const QString&) {
                    if (ok) syncWidgetGuards(SessionConnectionState::Connected);
                });
        connect(sessionPresenter_, &ModbusSessionPresenter::sessionConnected, this,
                [this]() { syncWidgetGuards(SessionConnectionState::Connected); });
        connect(sessionPresenter_, &ModbusSessionPresenter::sessionDisconnected, this,
                [this](const QString&) { syncWidgetGuards(SessionConnectionState::Disconnected); });
    }
}

void ModbusPagePresenter::teardownServices() {
    if (trafficLogController_) {
        delete trafficLogController_;
        trafficLogController_ = nullptr;
    }
    if (pollingController_) {
        delete pollingController_;
        pollingController_ = nullptr;
    }
    requestService_.reset();
    if (sessionPresenter_) {
        delete sessionPresenter_;
        sessionPresenter_ = nullptr;
    }
}

void ModbusPagePresenter::switchMode(SessionMode newMode) {
    if (newMode == mode_) {
        return;
    }
    pendingMode_ = newMode;
    if (sessionPresenter_) {
        connect(sessionPresenter_, &ModbusSessionPresenter::stackReleased,
                this, &ModbusPagePresenter::onStackReleasedForSwitch, Qt::UniqueConnection);
        if (sessionPresenter_->isSessionConnected()) {
            sessionPresenter_->requestDisconnect();
        } else {
            sessionPresenter_->releaseStack();
        }
        return;
    }

    teardownServices();
    mode_ = newMode;
    linked_ = false;
    lastSyncedState_.reset();
    createServices();
    wireConnections();
    if (view_) {
        view_->setProtocolType(modeDescriptor(mode_).protocolType);
    }
}

void ModbusPagePresenter::onStackReleasedForSwitch() {
    if (!sessionPresenter_) return;
    disconnect(sessionPresenter_, &ModbusSessionPresenter::stackReleased,
               this, &ModbusPagePresenter::onStackReleasedForSwitch);

    QMetaObject::invokeMethod(this, [this]() {
        teardownServices();
        mode_ = pendingMode_;
        linked_ = false;
        lastSyncedState_.reset();
        createServices();
        wireConnections();
        if (view_) {
            view_->setProtocolType(modeDescriptor(mode_).protocolType);
        }
    }, Qt::QueuedConnection);
}

void ModbusPagePresenter::setLinked(bool linked) {
    linked_ = linked;
    if (sessionPresenter_) {
        sessionPresenter_->setLinked(linked);
    }
    if (view_) {
        view_->setLinkageState(linked);
    }
}

bool ModbusPagePresenter::isLinked() const noexcept {
    return linked_;
}

ModbusSessionPresenter* ModbusPagePresenter::sessionPresenter() const noexcept {
    return sessionPresenter_;
}

PollingController* ModbusPagePresenter::pollingController() const noexcept {
    return pollingController_;
}

RequestSubmissionService* ModbusPagePresenter::requestService() const noexcept {
    return requestService_.get();
}

TrafficLogController* ModbusPagePresenter::trafficLogController() const noexcept {
    return trafficLogController_;
}

void ModbusPagePresenter::requestConnect(const ModbusConnectionSpec& spec) {
    if (!sessionPresenter_) {
        SPDLOG_WARN("ModbusPagePresenter: requestConnect dropped (no session presenter)");
        return;
    }
    if (sessionPresenter_->connectionState() == SessionConnectionState::Connecting) {
        SPDLOG_INFO("ModbusPagePresenter: connect request ignored (already connecting)");
        return;
    }
    sessionPresenter_->requestConnect(spec);
}

void ModbusPagePresenter::requestDisconnect() {
    if (sessionPresenter_) {
        sessionPresenter_->requestDisconnect();
    }
}

void ModbusPagePresenter::updateSettings(const ModbusTimingParams& params) {
    if (sessionPresenter_) {
        sessionPresenter_->updateSettings(params);
    }
}

bool ModbusPagePresenter::ensureConnected() {
    if (sessionPresenter_ && sessionPresenter_->isSessionConnected()) {
        return true;
    }
    const auto state = sessionPresenter_
        ? sessionPresenter_->connectionState()
        : SessionConnectionState::Disconnected;
    const QString msg = [state, this]() -> QString {
        switch (state) {
        case SessionConnectionState::Connecting:
            return tr("Request rejected: connection in progress, please wait.");
        case SessionConnectionState::Disconnecting:
            return tr("Request rejected: disconnecting in progress.");
        default:
            return tr("Request rejected: device not connected.");
        }
    }();
    if (trafficLogController_) {
        trafficLogController_->logInfo(msg);
    }
    if (view_) {
        view_->showStatusMessage(msg, true);
    }
    return false;
}

void ModbusPagePresenter::syncWidgetGuards(SessionConnectionState state) {
    const bool isPolling = pollingController_ ? pollingController_->isPollingActive() : false;
    if (lastSyncedState_.has_value() && *lastSyncedState_ == state && lastPollingActive_ == isPolling) {
        return;
    }
    lastSyncedState_ = state;
    lastPollingActive_ = isPolling;

    const bool isConnected = (state == SessionConnectionState::Connected);
    const bool isTransportConnected = (state == SessionConnectionState::TransportConnected);

    ExecutionGuardState guards;
    guards.pollingActive = isPolling;
    guards.rawSendEnabled = (isConnected || (mode_ != SessionMode::Tcp && isTransportConnected));
    guards.executeEnabled = (isConnected || isTransportConnected);
    guards.pollingEnabled = isConnected;

    if (state == SessionConnectionState::Connecting) {
        guards.tooltipMessage = tr("Connecting — please wait before sending commands.");
    } else if (state == SessionConnectionState::Disconnecting) {
        guards.tooltipMessage = tr("Disconnecting...");
    } else if (state == SessionConnectionState::TransportConnected) {
        if (mode_ == SessionMode::Tcp) {
            guards.tooltipMessage = tr("Device is establishing session. Read commands available; write commands locked until ready.");
        }
    } else if (!isConnected) {
        guards.tooltipMessage = tr("Connect device to send commands.");
    }

    if (view_) {
        view_->renderConnectionState(state);
        view_->setExecutionGuards(guards);
    }
}

void ModbusPagePresenter::executeRequest(const ModbusExecuteSpec& spec) {
    if (!ensureConnected()) {
        return;
    }
    if (spec.isRawMode) {
        handleRawExecute(spec);
        return;
    }
    handleStandardExecute(spec);
}

void ModbusPagePresenter::handleRawExecute(const ModbusExecuteSpec& spec) {
    if (!requestService_ || !requestService_->validateRawData(spec.rawPduOrAdu)) {
        if (trafficLogController_) {
            trafficLogController_->logError(tr("Error: Invalid raw frame format"));
        }
        return;
    }
    if (trafficLogController_) {
        trafficLogController_->logSendingRawData(spec.rawPduOrAdu);
    }
    if (sessionPresenter_) {
        sessionPresenter_->sendRaw(spec.rawPduOrAdu);
    }
}

void ModbusPagePresenter::handleStandardExecute(const ModbusExecuteSpec& spec) {
    if (!requestService_ || !sessionPresenter_) return;

    auto val = requestService_->validateSpec(spec, mode_);
    if (!val.isValid) {
        if (trafficLogController_) trafficLogController_->logError(val.errorMessage);
        if (view_) view_->showStatusMessage(val.errorMessage, true);
        return;
    }

    const auto kind = ::modbus::base::function_traits::isReadOperation(spec.functionCode)
        ? RequestKind::Read : RequestKind::Write;
    auto result = requestService_->buildExecuteRequest(spec, mode_, kind);
    if (!result.ok) {
        if (trafficLogController_) {
            trafficLogController_->logError(tr("Error: %1").arg(result.error));
        }
        return;
    }

    logExecuteSubmission(spec, kind, result.traceId);
    sessionPresenter_->submitRequest(result.pdu, spec.slaveId, result.requestId, result.traceId);
}

void ModbusPagePresenter::logExecuteSubmission(const ModbusExecuteSpec& spec, RequestKind kind, TraceId traceId) {
    if (!trafficLogController_) return;
    if (kind == RequestKind::Read) {
        trafficLogController_->logSendingReadRequest(
            spec.functionCode, spec.startAddress, spec.quantity, spec.slaveId, traceId);
    } else {
        trafficLogController_->logSendingWriteRequest(
            spec.functionCode, spec.startAddress, QString::fromUtf8(spec.writeData.toHex(' ')), spec.slaveId, traceId);
    }
}

void ModbusPagePresenter::togglePolling(bool start, const ModbusExecuteSpec& spec, bool userConfirmedWritePoll) {
    if (!start) {
        if (pollingController_) pollingController_->stopPoll();
        syncWidgetGuards(sessionPresenter_ ? sessionPresenter_->connectionState() : SessionConnectionState::Disconnected);
        return;
    }

    if (!ensureConnected()) {
        syncWidgetGuards(sessionPresenter_ ? sessionPresenter_->connectionState() : SessionConnectionState::Disconnected);
        return;
    }

    if (!validatePollingPreconditions(spec, userConfirmedWritePoll)) {
        syncWidgetGuards(sessionPresenter_->connectionState());
        return;
    }

    if (pollingController_) {
        PollSpec pollSpec;
        pollSpec.functionCode = spec.functionCode;
        pollSpec.startAddress = spec.startAddress;
        pollSpec.quantity = spec.quantity;
        pollSpec.slaveId = spec.slaveId;
        pollingController_->startPolling(pollSpec);
        syncWidgetGuards(sessionPresenter_->connectionState());
    }
}

bool ModbusPagePresenter::validatePollingPreconditions(const ModbusExecuteSpec& spec, bool userConfirmedWritePoll) {
    const bool isWrite = ::modbus::base::function_traits::isWriteOperation(spec.functionCode);
    if (isWrite) {
        if (spec.slaveId == 0 && (mode_ == SessionMode::Rtu || mode_ == SessionMode::Ascii)) {
            const QString err = tr("Broadcast write polling is strictly forbidden.");
            if (trafficLogController_) trafficLogController_->logError(err);
            if (view_) view_->showStatusMessage(err, true);
            return false;
        }
        if (!userConfirmedWritePoll) {
            const QString err = tr("Write polling rejected: confirmation required.");
            if (trafficLogController_) trafficLogController_->logWarning(err);
            if (view_) view_->showStatusMessage(err, true);
            return false;
        }
    }
    if (requestService_) {
        auto val = requestService_->validateSpec(spec, mode_);
        if (!val.isValid) {
            if (trafficLogController_) trafficLogController_->logError(val.errorMessage);
            if (view_) view_->showStatusMessage(val.errorMessage, true);
            return false;
        }
    }
    return true;
}

void ModbusPagePresenter::changePollInterval(int intervalMs) {
    if (pollingController_) {
        pollingController_->setPollingInterval(intervalMs);
    }
}

void ModbusPagePresenter::handleLogMessage(const QString& message, bool isError) {
    if (!trafficLogController_) return;
    if (isError) {
        trafficLogController_->logWarning(message);
    } else {
        trafficLogController_->logInfo(message);
    }
}

void ModbusPagePresenter::setRawFramesEnabled(bool enabled) {
    if (trafficLogController_) {
        trafficLogController_->setRawFramesEnabled(enabled);
    }
}

void ModbusPagePresenter::recordTx() {
    ++txCount_;
    notifyMetrics();
}

void ModbusPagePresenter::recordRx(int rttMs) {
    ++rxCount_;
    lastRttMs_ = rttMs;
    notifyMetrics();
}

void ModbusPagePresenter::recordError() {
    ++errCount_;
    notifyMetrics();
}

void ModbusPagePresenter::notifyMetrics() {
    if (view_) {
        view_->updateMetrics(txCount_, rxCount_, errCount_, lastRttMs_);
    }
}

void ModbusPagePresenter::handleRequestFinished(int requestId,
                                                const ::modbus::session::ModbusResponse& response) {
    if (!requestService_) return;

    auto trackingInfo = requestService_->lookupAndRemove(requestId);
    if (!trackingInfo.has_value()) {
        return;
    }

    auto kind = trackingInfo->kind;
    uint16_t addr = trackingInfo->address;
    const TraceId traceId = trackingInfo->traceId;

    const QString localizedError = (response.isError() || response.isBusy())
        ? formatModbusError(response) : QString();

    if (kind == RequestKind::Poll && pollingController_) {
        pollingController_->handleResponse(!response.isError(), response.rttMs,
                                            response.retryCount(), localizedError);
    }

    switch (response.kind) {
    case ::modbus::session::ModbusResponseKind::NoResponseExpected:
        if (trafficLogController_) trafficLogController_->logBroadcastWriteSuccess(response.retryCount(), traceId);
        break;
    case ::modbus::session::ModbusResponseKind::Success:
        recordRx(response.rttMs);
        if (kind == RequestKind::Read && trafficLogController_) {
            trafficLogController_->logReadSuccess(response.retryCount(), traceId);
        } else if (kind == RequestKind::Write && trafficLogController_) {
            trafficLogController_->logWriteSuccess(response.retryCount(), traceId);
        }
        break;
    case ::modbus::session::ModbusResponseKind::Error:
        if (response.isBusy()) {
            if (kind != RequestKind::Poll && trafficLogController_) trafficLogController_->logWarning(localizedError);
            break;
        }
        recordError();
        if (kind != RequestKind::Poll && trafficLogController_) {
            trafficLogController_->logRequestError(localizedError, response.retryCount(), traceId);
        }
        break;
    }

    if (!response.isError() && linked_) {
        emit linkageDataReceived(response.pdu, modeDescriptor(mode_).protocolType, addr);
    }
}

namespace {

QString formatExceptionCode(::modbus::base::ExceptionCode code) {
    switch (code) {
    case ::modbus::base::ExceptionCode::IllegalFunction:
        return QObject::tr("Illegal Function");
    case ::modbus::base::ExceptionCode::IllegalDataAddress:
        return QObject::tr("Illegal Data Address");
    case ::modbus::base::ExceptionCode::IllegalDataValue:
        return QObject::tr("Illegal Data Value");
    case ::modbus::base::ExceptionCode::ServerDeviceFailure:
        return QObject::tr("Server Device Failure");
    case ::modbus::base::ExceptionCode::Acknowledge:
        return QObject::tr("Acknowledge");
    case ::modbus::base::ExceptionCode::ServerDeviceBusy:
        return QObject::tr("Server Device Busy");
    case ::modbus::base::ExceptionCode::NegativeAcknowledge:
        return QObject::tr("Negative Acknowledge");
    case ::modbus::base::ExceptionCode::MemoryParityError:
        return QObject::tr("Memory Parity Error");
    case ::modbus::base::ExceptionCode::GatewayPathUnavailable:
        return QObject::tr("Gateway Path Unavailable");
    case ::modbus::base::ExceptionCode::GatewayTargetDeviceFailed:
        return QObject::tr("Gateway Target Device Failed To Respond");
    default:
        return QObject::tr("Unknown Exception");
    }
}

} // namespace

QString ModbusPagePresenter::formatModbusError(const ::modbus::session::ModbusResponse& response) {
    switch (response.errorCode) {
    case ::modbus::base::ModbusError::Timeout:
        return tr("Request timed out");
    case ::modbus::base::ModbusError::NotConnected:
        return tr("Not connected");
    case ::modbus::base::ModbusError::ConnectionFailed:
        return response.error.isEmpty() ? tr("Connection failed") : response.error;
    case ::modbus::base::ModbusError::Busy:
        return tr("Request already in progress");
    case ::modbus::base::ModbusError::InvalidAdu:
        return tr("Invalid response frame");
    case ::modbus::base::ModbusError::InvalidPdu:
        return response.error.isEmpty() ? tr("Invalid PDU parameters") : response.error;
    case ::modbus::base::ModbusError::CrcMismatch:
        return tr("CRC checksum error");
    case ::modbus::base::ModbusError::LrcMismatch:
        return tr("LRC checksum error");
    case ::modbus::base::ModbusError::ExceptionResponse:
        return tr("Modbus exception: %1 (0x%2)")
            .arg(formatExceptionCode(response.exceptionCode))
            .arg(static_cast<int>(response.exceptionCode), 2, 16, QChar('0'));
    case ::modbus::base::ModbusError::WriteFailed:
        return tr("Failed to send data");
    case ::modbus::base::ModbusError::Aborted:
        return tr("Request aborted");
    case ::modbus::base::ModbusError::TooManyInvalidBytes:
        return tr("Too many invalid response bytes");
    case ::modbus::base::ModbusError::None:
    case ::modbus::base::ModbusError::Unknown:
    default:
        return response.error.isEmpty() ? tr("Unknown error") : response.error;
    }
}

void ModbusPagePresenter::setTrafficLogControllerForTest(TrafficLogController* controller) {
    trafficLogController_ = controller;
}

} // namespace ui::application::modbus
