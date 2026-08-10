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
    [[maybe_unused]] const bool _conn = connectionStateMachine_->transitionTo(SessionConnectionState::Connecting);
    const quint64 generation = connectionGeneration_;

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    }

    initStack(config);
    if (!worker_ || !channel_) {
        [[maybe_unused]] const bool _disc = connectionStateMachine_->transitionTo(SessionConnectionState::Disconnected);
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
    [[maybe_unused]] const bool _conn = connectionStateMachine_->transitionTo(SessionConnectionState::Connecting);

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Opening %1...").arg(serialConfig.portName));
    }
    const quint64 generation = connectionGeneration_;

    initStack(modbusConfig);
    if (!worker_ || !channel_) {
        [[maybe_unused]] const bool _disc = connectionStateMachine_->transitionTo(SessionConnectionState::Disconnected);
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
    [[maybe_unused]] const bool _disc = connectionStateMachine_->transitionTo(SessionConnectionState::Disconnecting);
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
    const ModbusModeDescriptor descriptor = modeDescriptor(mode_);
    spdlog::info("ModbusSessionPresenter[{}]: Release requested (generation={})",
                 descriptor.logName,
                 static_cast<unsigned long long>(connectionGeneration_ + 1));
    if (pollingController_) pollingController_->reset();
    ++connectionGeneration_;
    suppressDisconnectAlert_ = true;
    [[maybe_unused]] const bool _disc = connectionStateMachine_->transitionTo(SessionConnectionState::Disconnected);
    const bool wasLinked = linked_;
    linked_ = false;
    if (controlWidget_) {
        controlWidget_->setLinked(false);
        controlWidget_->setPollingEnabled(false);
    }
    if (wasLinked) {
        emit linkageSourceDisconnected();
    }
    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Releasing Modbus stack..."));
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
        spdlog::info("ModbusSessionPresenter[{}]: Release completed immediately (no live stack)",
                     descriptor.logName);
        if (trafficLogController_) {
            trafficLogController_->logConnectionInfo(tr("Release completed"));
        }
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
                                           int requestId, TraceId traceId) {
    assertGuiThread("submitRequest must be called on the GUI thread");
    if (worker_) {
        worker_->submit(pdu, slaveId, requestId, traceId);
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
    spdlog::info("ModbusSessionPresenter[{}]: Release completed",
                 modeDescriptor(mode_).logName);
    if (!worker_ && !channel_ && !client_
        && connectionStateMachine_->currentState() != SessionConnectionState::Connecting) {
        syncConnectionWidget(SessionConnectionState::Disconnected);
    }
    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Release completed"));
    }
    emit stackReleased();
    maybeRunDeferredAction();
}

void ModbusSessionPresenter::onReleaseTimedOut(const QString& message) {
    assertGuiThread("onReleaseTimedOut must run on the GUI thread");
    spdlog::error("ModbusSessionPresenter[{}]: Release timed out: {}",
                  modeDescriptor(mode_).logName,
                  message.toStdString());
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
    auto stackOpt = ::modbus::factory::createStack(config);
    if (!stackOpt) {
        if (trafficLogController_) {
            trafficLogController_->logError(tr("Failed to create Modbus stack"));
        }
        return;
    }
    auto stack = std::move(*stackOpt);

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

SessionConnectionState ModbusSessionPresenter::deriveUiState(
    ::modbus::session::ConnectionStateMachine::State coreState,
    io::ChannelState channelState,
    ::modbus::session::SessionHealth health) {
    using Core = ::modbus::session::ConnectionStateMachine::State;
    switch (coreState) {
    case Core::Disconnected:
        return SessionConnectionState::Disconnected;
    case Core::Connecting:
        // Channel Open → TransportConnected (session not yet validated)
        if (channelState == io::ChannelState::Open) {
            return SessionConnectionState::TransportConnected;
        }
        return SessionConnectionState::Connecting;
    case Core::Connected:
        // Transport is up, but the Modbus session may not be healthy.
        // Only show "Connected" when the device has actually responded.
        if (health == ::modbus::session::SessionHealth::Healthy) {
            return SessionConnectionState::Connected;
        }
        return SessionConnectionState::TransportConnected;
    case Core::Reconnecting:
        return SessionConnectionState::Connecting;
    case Core::Disconnecting:
        return SessionConnectionState::Disconnecting;
    case Core::Failed:
        return SessionConnectionState::Disconnected;
    }
    return SessionConnectionState::Disconnected;
}

void ModbusSessionPresenter::syncStateFromCore() {
    assertGuiThread("syncStateFromCore must run on the GUI thread");
    if (!client_) {
        // No live client — stay in current state (release path handles this).
        return;
    }

    const auto coreState = client_->connectionState();
    // Channel state is needed for the Connecting vs TransportConnected
    // distinction; channel_ is on the IO thread but channel_->state() is a
    // simple enum read — safe for the same reason as client_->connectionState().
    const auto chanState = channel_ ? channel_->state() : io::ChannelState::Closed;
    const auto health = client_->sessionHealth();

    const auto derivedUi = deriveUiState(coreState, chanState, health);
    const auto currentUi = connectionStateMachine_->currentState();

    if (derivedUi != currentUi) {
        const auto oldUi = currentUi;
        if (!connectionStateMachine_->transitionTo(derivedUi)) {
            spdlog::warn("ModbusSessionPresenter: core-state-derived UI transition "
                         "{} -> {} rejected by UI FSM rules; forcing disconnect",
                         SessionConnectionStateMachine::stateName(oldUi),
                         SessionConnectionStateMachine::stateName(derivedUi));
            // Core is authoritative — force the UI into Disconnected as a
            // safe fallback.
            connectionStateMachine_->forceTransitionTo(
                SessionConnectionState::Disconnected);
        }
    }
}

void ModbusSessionPresenter::handleChannelStateTransition(io::ChannelState state,
                                                          quint64 generation) {
    assertGuiThread("handleChannelStateTransition must run on the GUI thread");
    Q_UNUSED(generation);

    // Core FSM is the single source of truth — derive UI state from it.
    syncStateFromCore();

    const auto currentState = connectionStateMachine_->currentState();
    const bool isTcp = modeDescriptor(mode_).transportUiMode == TransportUiMode::Tcp;

    switch (state) {
    case io::ChannelState::Opening:
        if (currentState != SessionConnectionState::Connected) {
            syncConnectionWidget(SessionConnectionState::Connecting);
        }
        return;
    case io::ChannelState::Open:
        if (currentState != SessionConnectionState::Connected && isTcp && trafficLogController_) {
            trafficLogController_->logConnectionInfo(tr("Transport connected, validating session..."));
        }
        return;
    case io::ChannelState::Closing:
        syncConnectionWidget(SessionConnectionState::Disconnecting);
        return;
    case io::ChannelState::Closed:
    case io::ChannelState::Error: {
        const bool wasConnected = (currentState == SessionConnectionState::Connected);
        if (wasConnected || currentState != SessionConnectionState::Disconnected) {
            const bool shouldShowDisconnectAlert = isTcp && wasConnected && !suppressDisconnectAlert_;
            if (controlWidget_) {
                controlWidget_->setPollingEnabled(false);
            }
            // Attribute the disconnection reason: prefer the last channel
            // error, fall back to a generic message.
            const QString reason = client_
                ? client_->lastChannelError()
                : QString();
            if (isTcp && trafficLogController_) {
                const QString logMsg = reason.isEmpty()
                    ? tr("Disconnected")
                    : tr("Disconnected: %1").arg(reason);
                trafficLogController_->logConnectionInfo(logMsg);
            }
            if (shouldShowDisconnectAlert) {
                ui::common::connection_alert::showDisconnected(qApp->activeWindow());
            }
            emit sessionDisconnected(reason);
        }
        return;
    }
    }
}

void ModbusSessionPresenter::handleConnectFinished(bool ok, const QString& error,
                                                    quint64 generation) {
    assertGuiThread("handleConnectFinished must run on the GUI thread");
    if (generation != connectionGeneration_) return;

    // Core FSM is the single source of truth — derive UI state from it.
    syncStateFromCore();

    if (!ok) {
        // syncStateFromCore already derived the correct state.  Handle
        // side effects from the presenter layer.
        if (controlWidget_) {
            controlWidget_->setPollingEnabled(false);
        }
        if (pollingController_) {
            pollingController_->stopPoll();
        }
        if (trafficLogController_) {
            trafficLogController_->logConnectionInfo(tr("Connection failed: %1").arg(error));
        }
        emit sessionDisconnected(error);
        emit connectFinished(false, error);
        return;
    }

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Transport connected, waiting for device response..."));
    }
    emit sessionConnected();
    emit connectFinished(true, error);
}

void ModbusSessionPresenter::handleRequestFinished(int requestId,
                                                     const ::modbus::session::ModbusResponse& response,
                                                     quint64 generation) {
    assertGuiThread("handleRequestFinished must run on the GUI thread");
    if (generation != connectionGeneration_) return;
    // Re-derive UI state after each request: health may have changed
    // (Healthy <-> Unresponsive), which affects Connected vs TransportConnected.
    syncStateFromCore();
    if (!requestService_) return;
    emit requestFinished(requestId, response);
}

} // namespace ui::application::modbus
