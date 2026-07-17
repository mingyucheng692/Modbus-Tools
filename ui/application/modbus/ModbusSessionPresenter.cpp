#include "ModbusSessionPresenter.h"
#include "WorkerReleaseCoordinator.h"
#include "RequestSubmissionService.h"
#include "PollingController.h"
#include "TrafficLogController.h"
#include "../../widgets/ControlWidget.h"
#include "../../widgets/BaseConnectionWidget.h"
#include "../../common/ConnectionAlert.h"
#include "modbus/factory/ModbusFactory.h"
#include "modbus/session/ModbusClient.h"
#include <QMetaObject>
#include <QPointer>
#include <QApplication>
#include <QThread>
#include <spdlog/spdlog.h>

namespace ui::application::modbus {

namespace {

void assertObjectThread(const QObject* object, const char* context) {
    Q_ASSERT_X(object && QThread::currentThread() == object->thread(),
               "ModbusSessionPresenter",
               context);
}

} // namespace

ModbusSessionPresenter::ModbusSessionPresenter(SessionMode mode,
                                               QObject* parent)
    : QObject(parent),
      mode_(mode),
      timeoutMs_(config::Modbus::kDefaultTimeoutMs),
      retries_(0),
      retryIntervalMs_(config::Modbus::kDefaultRetryIntervalMs) {
    releaseCoordinator_ = std::make_unique<WorkerReleaseCoordinator>(this);
    connect(releaseCoordinator_.get(), &WorkerReleaseCoordinator::releaseCompleted,
            this, &ModbusSessionPresenter::onReleaseCompleted);
    connect(releaseCoordinator_.get(), &WorkerReleaseCoordinator::releaseTimedOut,
            this, &ModbusSessionPresenter::onReleaseTimedOut);

    connectionStateMachine_ = std::make_unique<SessionConnectionStateMachine>(this);
    // stateChanged is emitted synchronously from transitionTo (same thread),
    // so onConnectionStateChanged applies suppress/sync side effects inline
    // before control returns to the caller.
    connect(connectionStateMachine_.get(), &SessionConnectionStateMachine::stateChanged,
            this, &ModbusSessionPresenter::onConnectionStateChanged, Qt::DirectConnection);
}

ModbusSessionPresenter::~ModbusSessionPresenter() {
    shutdown();
}

void ModbusSessionPresenter::connectTcp(const QString& ip, int port,
                                         const ::modbus::base::ModbusConfig& config) {
    ModbusConnectionSpec spec;
    spec.config = config;
    requestConnect(spec);
}

void ModbusSessionPresenter::startTcpConnect(const QString& ip, int port,
                                             const ::modbus::base::ModbusConfig& config) {
    assertGuiThread("startTcpConnect must run on the GUI thread");
    Q_ASSERT(mode_ == SessionMode::Tcp);
    spdlog::info("ModbusSessionPresenter[TCP]: Connect requested to {}:{}", ip.toStdString(), port);
    // suppressDisconnectAlert_ is reset by the Connecting state-entry handler.
    connectionStateMachine_->transitionTo(SessionConnectionState::Connecting);
    const quint64 generation = connectionGeneration_;

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    }

    initStack(config);
    if (!worker_ || !channel_) {
        connectionStateMachine_->transitionTo(SessionConnectionState::Disconnected);
        emit connectFinished(false, tr("Failed to create Modbus stack"));
        return;
    }

    activateStack(generation);
}

void ModbusSessionPresenter::requestConnect(const ModbusConnectionSpec& spec) {
    assertGuiThread("requestConnect must be called on the GUI thread");
    if (hasLiveOrPendingStack()) {
        deferredAction_ = [this, spec]() {
            startConnect(spec);
        };
        shutdown();
        return;
    }
    startConnect(spec);
}

void ModbusSessionPresenter::startConnect(const ModbusConnectionSpec& spec) {
    assertGuiThread("startConnect must run on the GUI thread");
    const ModbusModeDescriptor descriptor = modeDescriptor(spec.config.mode);
    Q_ASSERT(descriptor.sessionMode == mode_);

    if (descriptor.usesSerialConnection) {
        Q_ASSERT(spec.serialConfig.has_value());
        startSerialConnect(*spec.serialConfig, spec.config);
        return;
    }

    startTcpConnect(spec.config.ipAddress, spec.config.port, spec.config);
}

void ModbusSessionPresenter::connectRtu(const io::SerialConfig& serialConfig,
                                         const ::modbus::base::ModbusConfig& modbusConfig) {
    ModbusConnectionSpec spec;
    spec.config = modbusConfig;
    spec.serialConfig = serialConfig;
    requestConnect(spec);
}

void ModbusSessionPresenter::startSerialConnect(const io::SerialConfig& serialConfig,
                                                const ::modbus::base::ModbusConfig& modbusConfig) {
    assertGuiThread("startSerialConnect must run on the GUI thread");
    const ModbusModeDescriptor descriptor = modeDescriptor(modbusConfig.mode);
    Q_ASSERT(descriptor.sessionMode == mode_);
    spdlog::info("ModbusSessionPresenter[{}]: Connect requested to {}",
                 descriptor.logName,
                 serialConfig.portName.toStdString());
    connectionStateMachine_->transitionTo(SessionConnectionState::Connecting);

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Opening %1...").arg(serialConfig.portName));
    }
    const quint64 generation = connectionGeneration_;

    initStack(modbusConfig);
    if (!worker_ || !channel_) {
        connectionStateMachine_->transitionTo(SessionConnectionState::Disconnected);
        emit connectFinished(false, tr("Failed to create Modbus stack"));
        return;
    }

    activateStack(generation);
}

void ModbusSessionPresenter::activateStack(quint64 generation) {
    assertGuiThread("activateStack must run on the GUI thread");
    currentConfig_.timeoutMs = timeoutMs_;
    currentConfig_.retries = retries_;
    currentConfig_.retryIntervalMs = retryIntervalMs_;
    worker_->updateConfig(currentConfig_);

    setupChannelMonitor(generation);
    setupChannelStateHandler(generation);
    setupWorkerSignals(generation);

    if (channelThread_ && !channelThread_->isRunning()) {
        channelThread_->start();
    }
    worker_->start();
    worker_->requestConnect();
}

void ModbusSessionPresenter::requestDisconnect() {
    assertGuiThread("requestDisconnect must be called on the GUI thread");
    const ModbusModeDescriptor descriptor = modeDescriptor(mode_);
    spdlog::info("ModbusSessionPresenter[{}]: Disconnect requested",
                 descriptor.logName);
    deferredAction_ = nullptr;
    // suppressDisconnectAlert_ is set by the Disconnecting state-entry handler.
    connectionStateMachine_->transitionTo(SessionConnectionState::Disconnecting);
    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Disconnecting..."));
    }
    shutdown();
}

void ModbusSessionPresenter::shutdown() {
    assertGuiThread("shutdown must be called on the GUI thread");
    requestRelease(tr("Shutdown timed out; restart recommended"));
}

void ModbusSessionPresenter::releaseStack() {
    assertGuiThread("releaseStack must be called on the GUI thread");
    deferredAction_ = nullptr;
    requestRelease(tr("Release timed out; restart recommended"));
}

bool ModbusSessionPresenter::hasLiveOrPendingStack() const {
    return worker_ || channel_ || client_ || channelThread_ || modbusWorkerThread_
           || (releaseCoordinator_ && releaseCoordinator_->hasPending());
}

void ModbusSessionPresenter::requestRelease(const QString& timeoutMessage) {
    assertGuiThread("requestRelease must run on the GUI thread");
    if (pollingController_) pollingController_->reset();
    ++connectionGeneration_;
    suppressDisconnectAlert_ = true;
    connectionStateMachine_->transitionTo(SessionConnectionState::Disconnected);
    const bool wasLinked = linked_;
    linked_ = false;
    if (controlWidget_) {
        controlWidget_->setLinked(false);
        controlWidget_->setPollingEnabled(false);
    }
    if (wasLinked) {
        emit linkageSourceDisconnected();
    }

    // Move the live stack into the coordinator. After this move the
    // Presenter's own pointers are nullptr; the coordinator owns the lifetime
    // until all threads join or the bounded timeout fires.
    StackHandle handle;
    handle.channel = std::move(channel_);
    handle.client = std::move(client_);
    handle.worker = std::move(worker_);
    handle.channelThread = std::move(channelThread_);
    handle.workerThread = std::move(modbusWorkerThread_);

    if (handle.empty()) {
        // Nothing live: complete synchronously. This preserves the original
        // contract that an empty release path emits stackReleased() inline so
        // callers (and tests) observing the signal without pumping the event
        // loop still see it. Widget sync already applied by the
        // Disconnected state-entry handler during transitionTo above.
        emit stackReleased();
        maybeRunDeferredAction();
        return;
    }

    syncConnectionWidget(SessionConnectionState::Disconnecting);
    releaseCoordinator_->requestRelease(std::move(handle), timeoutMessage);
}

void ModbusSessionPresenter::updateSettings(const ModbusTimingParams& params) {
    assertGuiThread("updateSettings must be called on the GUI thread");
    timeoutMs_ = static_cast<int>(params.timeout.count());
    retries_ = params.retryCount;
    retryIntervalMs_ = static_cast<int>(params.retryInterval.count());
    if (worker_) {
        currentConfig_.timeoutMs = timeoutMs_;
        currentConfig_.retries = retries_;
        currentConfig_.retryIntervalMs = retryIntervalMs_;
        worker_->updateConfig(currentConfig_);
    }
}

bool ModbusSessionPresenter::isSessionConnected() const {
    return connectionStateMachine_->currentState() == SessionConnectionState::Connected;
}

quint64 ModbusSessionPresenter::connectionGeneration() const {
    return connectionGeneration_;
}

SessionMode ModbusSessionPresenter::mode() const {
    return mode_;
}

void ModbusSessionPresenter::submitRequest(const ::modbus::base::Pdu& pdu, int slaveId,
                                           int requestId) {
    assertGuiThread("submitRequest must be called on the GUI thread");
    if (worker_) {
        worker_->submit(pdu, slaveId, requestId);
    }
}

void ModbusSessionPresenter::sendRaw(const QByteArray& data) {
    assertGuiThread("sendRaw must be called on the GUI thread");
    if (worker_) {
        worker_->sendRaw(data);
    }
}

void ModbusSessionPresenter::setTrafficLogController(TrafficLogController* controller) {
    assertGuiThread("setTrafficLogController must be called on the GUI thread");
    trafficLogController_ = controller;
}

void ModbusSessionPresenter::setPollingController(PollingController* controller) {
    assertGuiThread("setPollingController must be called on the GUI thread");
    if (pollingController_ == controller) {
        return;
    }

    if (pollingController_) {
        disconnect(this, &ModbusSessionPresenter::sessionConnected,
                   pollingController_, &PollingController::handleSessionConnected);
        disconnect(this, &ModbusSessionPresenter::sessionDisconnected,
                   pollingController_, &PollingController::handleSessionDisconnected);
    }

    pollingController_ = controller;
    if (!pollingController_) {
        return;
    }

    connect(this, &ModbusSessionPresenter::sessionConnected,
            pollingController_, &PollingController::handleSessionConnected);
    connect(this, &ModbusSessionPresenter::sessionDisconnected,
            pollingController_, &PollingController::handleSessionDisconnected);
    pollingController_->setSessionConnected(isSessionConnected());
}

void ModbusSessionPresenter::setRequestService(RequestSubmissionService* service) {
    assertGuiThread("setRequestService must be called on the GUI thread");
    requestService_ = service;
}

void ModbusSessionPresenter::setConnectionWidget(ui::widgets::BaseConnectionWidget* widget) {
    assertGuiThread("setConnectionWidget must be called on the GUI thread");
    connectionWidget_ = widget;
    syncConnectionWidget(connectionStateMachine_->currentState());
}

void ModbusSessionPresenter::setControlWidget(ui::widgets::ControlWidget* widget) {
    assertGuiThread("setControlWidget must be called on the GUI thread");
    controlWidget_ = widget;
    if (controlWidget_) {
        controlWidget_->setLinked(linked_);
        controlWidget_->setPollingEnabled(isSessionConnected());
    }
}

void ModbusSessionPresenter::assertGuiThread(const char* context) const {
    assertObjectThread(this, context);
}

void ModbusSessionPresenter::onConnectionStateChanged(SessionConnectionState state) {
    assertGuiThread("onConnectionStateChanged must run on the GUI thread");
    // State-driven derived flags. suppressDisconnectAlert_ is path-dependent
    // for Disconnected (release-initiated vs channel-error) and is therefore
    // set explicitly by the release path before transitioning, not here.
    switch (state) {
    case SessionConnectionState::Connecting:
    case SessionConnectionState::Connected:
        suppressDisconnectAlert_ = false;
        break;
    case SessionConnectionState::Disconnecting:
        suppressDisconnectAlert_ = true;
        break;
    case SessionConnectionState::Disconnected:
    case SessionConnectionState::TransportConnected:
        break; // path-dependent / intermediate — do not touch suppress here
    }
    // TransportConnected has a conditional UI sync (only when not already
    // Connected) applied by the channel-Open handler, so do not sync here.
    if (state != SessionConnectionState::TransportConnected) {
        syncConnectionWidget(state);
    }
}

void ModbusSessionPresenter::syncConnectionWidget(SessionConnectionState state) {
    assertGuiThread("syncConnectionWidget must run on the GUI thread");
    if (!connectionWidget_) {
        return;
    }

    switch (state) {
    case SessionConnectionState::Disconnected:
        connectionWidget_->setConnected(false);
        return;
    case SessionConnectionState::Connected:
        connectionWidget_->setConnected(true);
        return;
    case SessionConnectionState::Connecting:
        connectionWidget_->setDisplayState(ui::widgets::DisplayState::Connecting);
        return;
    case SessionConnectionState::TransportConnected:
        connectionWidget_->setDisplayState(ui::widgets::DisplayState::TransportConnected);
        return;
    case SessionConnectionState::Disconnecting:
        connectionWidget_->setDisplayState(ui::widgets::DisplayState::Disconnecting);
        return;
    }
}

void ModbusSessionPresenter::onReleaseCompleted() {
    assertGuiThread("onReleaseCompleted must run on the GUI thread");
    if (!worker_ && !channel_ && !client_
        && connectionStateMachine_->currentState() != SessionConnectionState::Connecting) {
        syncConnectionWidget(SessionConnectionState::Disconnected);
    }
    emit stackReleased();
    maybeRunDeferredAction();
}

void ModbusSessionPresenter::onReleaseTimedOut(const QString& message) {
    assertGuiThread("onReleaseTimedOut must run on the GUI thread");
    if (trafficLogController_) {
        trafficLogController_->logError(message);
    }
    emit sessionDisconnected(message);
}

void ModbusSessionPresenter::maybeRunDeferredAction() {
    assertGuiThread("maybeRunDeferredAction must run on the GUI thread");
    if (releaseCoordinator_ && !releaseCoordinator_->hasPending() && deferredAction_) {
        auto action = std::move(deferredAction_);
        deferredAction_ = nullptr;
        QMetaObject::invokeMethod(this, [action = std::move(action)]() mutable {
            if (action) {
                action();
            }
        }, Qt::QueuedConnection);
    }
}

void ModbusSessionPresenter::setLinked(bool linked) {
    assertGuiThread("setLinked must be called on the GUI thread");
    linked_ = linked;
    if (controlWidget_) {
        controlWidget_->setLinked(linked);
    }
}

bool ModbusSessionPresenter::isLinked() const {
    return linked_;
}

void ModbusSessionPresenter::initStack(const ::modbus::base::ModbusConfig& config) {
    assertGuiThread("initStack must run on the GUI thread");
    auto stack = ::modbus::factory::createStack(config);
    if (!stack.worker || !stack.thread || !stack.ioThread) {
        if (trafficLogController_) {
            trafficLogController_->logError(tr("Failed to create Modbus stack"));
        }
        return;
    }

    currentConfig_ = config;
    channel_ = std::move(stack.channel);
    client_ = std::move(stack.client);
    worker_ = std::move(stack.worker);
    channelThread_ = std::move(stack.ioThread);
    modbusWorkerThread_ = std::move(stack.thread);
}

void ModbusSessionPresenter::setupChannelMonitor(quint64 generation) {
    assertGuiThread("setupChannelMonitor must run on the GUI thread");
    QPointer<ModbusSessionPresenter> guard(this);
    channel_->setMonitor([guard, generation](bool isTx, const QByteArray& data) {
        if (!guard) return;
        QMetaObject::invokeMethod(guard.data(), [guard, generation, isTx, data]() {
            if (!guard) return;
            if (generation != guard->connectionGeneration_) return;
            if (guard->trafficLogController_) {
                guard->trafficLogController_->logRawFrame(
                    isTx ? ui::common::TrafficDirection::Tx : ui::common::TrafficDirection::Rx,
                    data);
            }
            emit guard->rawFrameReceived(isTx, data);
        }, Qt::QueuedConnection);
    });
}

void ModbusSessionPresenter::setupChannelStateHandler(quint64 generation) {
    assertGuiThread("setupChannelStateHandler must run on the GUI thread");
    QPointer<ModbusSessionPresenter> guard(this);
    channel_->addStateHandler([guard, generation](io::ChannelState state) {
        if (!guard) return;
        QMetaObject::invokeMethod(guard.data(), [guard, generation, state]() {
            if (!guard) return;
            if (generation != guard->connectionGeneration_) return;
            guard->handleChannelStateTransition(state, generation);
        }, Qt::QueuedConnection);
    });
}

void ModbusSessionPresenter::setupWorkerSignals(quint64 generation) {
    assertGuiThread("setupWorkerSignals must run on the GUI thread");
    QPointer<ModbusSessionPresenter> guard(this);

    connect(worker_.get(), &::modbus::dispatch::ModbusWorker::connectFinished, this,
        [this, guard, generation](bool ok, const QString& error) {
            if (!guard) return;
            handleConnectFinished(ok, error, generation);
        }, Qt::QueuedConnection);

    connect(worker_.get(), &::modbus::dispatch::ModbusWorker::requestFinished, this,
        [this, guard, generation](int requestId, const ::modbus::session::ModbusResponse& response) {
            if (!guard) return;
            handleRequestFinished(requestId, response, generation);
        }, Qt::QueuedConnection);
}

void ModbusSessionPresenter::handleChannelStateTransition(io::ChannelState state,
                                                       quint64 generation) {
    assertGuiThread("handleChannelStateTransition must run on the GUI thread");
    Q_UNUSED(generation);
    const bool hadLiveTransport = connectionStateMachine_->currentState() != SessionConnectionState::Disconnected;
    const bool isTcp = modeDescriptor(mode_).transportUiMode == TransportUiMode::Tcp;

    switch (state) {
    case io::ChannelState::Opening:
        if (connectionStateMachine_->currentState() != SessionConnectionState::Connected) {
            syncConnectionWidget(SessionConnectionState::Connecting);
        }
        return;
    case io::ChannelState::Open: {
        const bool wasNotConnected = (connectionStateMachine_->currentState() != SessionConnectionState::Connected);
        connectionStateMachine_->transitionTo(SessionConnectionState::TransportConnected);
        if (wasNotConnected) {
            syncConnectionWidget(connectionStateMachine_->currentState());
            if (isTcp && trafficLogController_) {
                trafficLogController_->logConnectionInfo(tr("Transport connected, validating session..."));
            }
        }
        return;
    }
    case io::ChannelState::Closing:
        syncConnectionWidget(SessionConnectionState::Disconnecting);
        return;
    case io::ChannelState::Closed:
    case io::ChannelState::Error: {
        const bool wasConnected = (connectionStateMachine_->currentState() == SessionConnectionState::Connected);
        connectionStateMachine_->transitionTo(SessionConnectionState::Disconnected);
        if (wasConnected || hadLiveTransport) {
            const bool shouldShowDisconnectAlert = isTcp && wasConnected && !suppressDisconnectAlert_;
            // Widget already synced to Disconnected by the state-entry handler
            // during transitionTo above (or no-op if already Disconnected).
            if (controlWidget_) {
                controlWidget_->setPollingEnabled(false);
            }
            if (isTcp && trafficLogController_) {
                trafficLogController_->logConnectionInfo(tr("Disconnected"));
            }
            if (shouldShowDisconnectAlert) {
                ui::common::connection_alert::showDisconnected(qApp->activeWindow());
            }
            emit sessionDisconnected(QString());
        }
        return;
    }
    }
}

void ModbusSessionPresenter::handleConnectFinished(bool ok, const QString& error,
                                                    quint64 generation) {
    assertGuiThread("handleConnectFinished must run on the GUI thread");
    if (generation != connectionGeneration_) return;

    if (!ok) {
        connectionStateMachine_->transitionTo(SessionConnectionState::Disconnected);
        if (controlWidget_) {
            controlWidget_->setPollingEnabled(false);
        }
        if (pollingController_) {
            pollingController_->stopPoll();
        }
        if (trafficLogController_) {
            trafficLogController_->logConnectionInfo(tr("Connection failed: %1").arg(error));
        }
        emit connectFinished(false, error);
        return;
    }

    // suppressDisconnectAlert_ is reset by the Connected state-entry handler.
    connectionStateMachine_->transitionTo(SessionConnectionState::Connected);
    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Connected"));
    }
    emit sessionConnected();
    emit connectFinished(true, error);
}

void ModbusSessionPresenter::handleRequestFinished(int requestId,
                                                     const ::modbus::session::ModbusResponse& response,
                                                     quint64 generation) {
    assertGuiThread("handleRequestFinished must run on the GUI thread");

    if (generation != connectionGeneration_) return;
    if (!requestService_) return;

    auto trackingInfo = requestService_->lookup(requestId);
    if (!trackingInfo.has_value()) {
        return;
    }

    auto kind = trackingInfo->kind;
    uint16_t addr = trackingInfo->address;

    if (kind == RequestKind::Poll) {
        if (pollingController_) {
            pollingController_->handleResponse(!response.isError(),
                                                response.rttMs,
                                                response.retryCount(),
                                                response.error);
        }
    }

    switch (response.kind) {
    case ::modbus::session::ModbusResponseKind::NoResponseExpected:
        if (trafficLogController_) {
            trafficLogController_->logBroadcastWriteSuccess(response.retryCount());
        }
        break;
    case ::modbus::session::ModbusResponseKind::Success:
        if (controlWidget_) {
            controlWidget_->recordRx(response.rttMs);
        }

        if (kind == RequestKind::Read && trafficLogController_) {
            trafficLogController_->logReadSuccess(response.retryCount());
        } else if (kind == RequestKind::Write && trafficLogController_) {
            trafficLogController_->logWriteSuccess(response.retryCount());
        }
        break;
    case ::modbus::session::ModbusResponseKind::Error:
        if (response.isBusy()) {
            // Soft-lock contention is not a protocol error: don't bump the
            // error counter; surface a dedicated warning so the UI shows a
            // precise "request in progress" hint instead of a generic error.
            if (kind != RequestKind::Poll && trafficLogController_) {
                trafficLogController_->logWarning(response.error);
            }
            break;
        }
        if (controlWidget_) {
            controlWidget_->recordError();
        }

        if (kind != RequestKind::Poll && trafficLogController_) {
            trafficLogController_->logRequestError(response.error, response.retryCount());
        }
        break;
    }

    emit requestFinished(requestId, response);
}

} // namespace ui::application::modbus
