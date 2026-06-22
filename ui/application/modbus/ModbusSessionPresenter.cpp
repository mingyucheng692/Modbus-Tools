#include "ModbusSessionPresenter.h"
#include "RequestSubmissionService.h"
#include "PollingController.h"
#include "TrafficLogController.h"
#include "../../widgets/ControlWidget.h"
#include "../../widgets/BaseConnectionWidget.h"
#include "../../widgets/SerialConnectionWidget.h"
#include "../../common/ConnectionAlert.h"
#include "modbus/factory/ModbusFactory.h"
#include <QMetaObject>
#include <QPointer>
#include <QTimer>
#include <QApplication>
#include <QThread>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace ui::application::modbus {

namespace {

void assertObjectThread(const QObject* object, const char* context) {
    Q_ASSERT_X(object && QThread::currentThread() == object->thread(),
               "ModbusSessionPresenter",
               context);
}

void setConnectionWidgetDisplayStateImpl(QWidget* connectionWidget,
                                         SessionMode mode,
                                         ui::widgets::BaseConnectionWidget::DisplayState displayState) {
    if (!connectionWidget) {
        return;
    }

    if (mode == SessionMode::Tcp) {
        if (auto* connWidget = qobject_cast<ui::widgets::BaseConnectionWidget*>(connectionWidget)) {
            QMetaObject::invokeMethod(connWidget,
                                      [connWidget, displayState]() {
                                          connWidget->setDisplayState(displayState);
                                      },
                                      Qt::QueuedConnection);
        }
        return;
    }

    if (auto* serialWidget = qobject_cast<ui::widgets::SerialConnectionWidget*>(connectionWidget)) {
        QMetaObject::invokeMethod(serialWidget,
                                  [serialWidget, displayState]() {
                                      serialWidget->setDisplayState(displayState);
                                  },
                                  Qt::QueuedConnection);
    }
}

} // namespace

ModbusSessionPresenter::ModbusSessionPresenter(SessionMode mode, QObject* parent)
    : QObject(parent),
      mode_(mode),
      timeoutMs_(app::constants::Values::Modbus::kDefaultTimeoutMs),
      retries_(0),
      retryIntervalMs_(app::constants::Values::Modbus::kDefaultRetryIntervalMs) {
}

ModbusSessionPresenter::~ModbusSessionPresenter() {
    shutdown();
}

void ModbusSessionPresenter::connectTcp(const QString& ip, int port,
                                         const ::modbus::base::ModbusConfig& config) {
    assertGuiThread("connectTcp must be called on the GUI thread");
    if (hasLiveOrPendingStack()) {
        deferredAction_ = [this, ip, port, config]() {
            startTcpConnect(ip, port, config);
        };
        shutdown();
        return;
    }
    startTcpConnect(ip, port, config);
}

void ModbusSessionPresenter::startTcpConnect(const QString& ip, int port,
                                             const ::modbus::base::ModbusConfig& config) {
    assertGuiThread("startTcpConnect must run on the GUI thread");
    Q_ASSERT(mode_ == SessionMode::Tcp);
    spdlog::info("ModbusSessionPresenter[TCP]: Connect requested to {}:{}", ip.toStdString(), port);
    suppressDisconnectAlert_ = false;
    applyConnectionState(SessionConnectionState::Connecting);
    const quint64 generation = connectionGeneration_;

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    }

    initStack(config);
    if (!worker_ || !channel_) {
        applyConnectionState(SessionConnectionState::Disconnected);
        emit connectFinished(false, tr("Failed to create Modbus stack"));
        return;
    }

    currentConfig_.timeoutMs = timeoutMs_;
    currentConfig_.retries = retries_;
    currentConfig_.retryIntervalMs = retryIntervalMs_;
    worker_->updateConfig(currentConfig_);

    setupChannelMonitor(generation);
    setupChannelStateHandler(generation);
    setupWorkerSignals(generation);

    if (ioThread_ && !ioThread_->isRunning()) {
        ioThread_->start();
    }
    worker_->start();
    worker_->requestConnect();
}

void ModbusSessionPresenter::connectRtu(const io::SerialConfig& serialConfig,
                                         const ::modbus::base::ModbusConfig& modbusConfig) {
    assertGuiThread("connectRtu must be called on the GUI thread");
    if (hasLiveOrPendingStack()) {
        deferredAction_ = [this, serialConfig, modbusConfig]() {
            startRtuConnect(serialConfig, modbusConfig);
        };
        shutdown();
        return;
    }
    startRtuConnect(serialConfig, modbusConfig);
}

void ModbusSessionPresenter::startRtuConnect(const io::SerialConfig& serialConfig,
                                             const ::modbus::base::ModbusConfig& modbusConfig) {
    assertGuiThread("startRtuConnect must run on the GUI thread");
    Q_ASSERT(mode_ == SessionMode::Rtu);
    spdlog::info("ModbusSessionPresenter[RTU]: Connect requested to {}", serialConfig.portName.toStdString());
    applyConnectionState(SessionConnectionState::Connecting);

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Opening %1...").arg(serialConfig.portName));
    }
    const quint64 generation = connectionGeneration_;

    initStack(modbusConfig);
    if (!worker_ || !channel_) {
        applyConnectionState(SessionConnectionState::Disconnected);
        emit connectFinished(false, tr("Failed to create Modbus stack"));
        return;
    }

    currentConfig_.timeoutMs = timeoutMs_;
    currentConfig_.retries = retries_;
    currentConfig_.retryIntervalMs = retryIntervalMs_;
    worker_->updateConfig(currentConfig_);

    setupChannelMonitor(generation);
    setupChannelStateHandler(generation);
    setupWorkerSignals(generation);

    if (ioThread_ && !ioThread_->isRunning()) {
        ioThread_->start();
    }
    worker_->start();
    worker_->requestConnect();
}

void ModbusSessionPresenter::requestDisconnect() {
    assertGuiThread("requestDisconnect must be called on the GUI thread");
    spdlog::info("ModbusSessionPresenter[{}]: Disconnect requested",
                 mode_ == SessionMode::Tcp ? "TCP" : "RTU");
    deferredAction_ = nullptr;
    suppressDisconnectAlert_ = true;
    applyConnectionState(SessionConnectionState::Disconnecting);
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
    return worker_ || channel_ || client_ || ioThread_ || workerThread_ || !pendingReleases_.empty();
}

void ModbusSessionPresenter::requestRelease(const QString& timeoutMessage) {
    assertGuiThread("requestRelease must run on the GUI thread");
    if (pollingController_) pollingController_->reset();
    ++connectionGeneration_;
    suppressDisconnectAlert_ = true;
    connectionState_ = SessionConnectionState::Disconnected;
    const bool wasLinked = linked_;
    linked_ = false;
    if (controlWidget_) {
        controlWidget_->setLinked(false);
        controlWidget_->setPollingEnabled(false);
    }
    if (wasLinked) {
        emit linkageSourceDisconnected();
    }

    auto release = std::make_shared<PendingReleaseContext>();
    release->channel = std::move(channel_);
    release->client = std::move(client_);
    release->worker = std::move(worker_);
    release->ioThread = std::move(ioThread_);
    release->thread = std::move(workerThread_);
    release->timeoutMessage = timeoutMessage;
    release->workerStopped = !release->worker;
    release->ioThreadFinished = !release->ioThread || !release->ioThread->isRunning();
    release->threadFinished = !release->thread || !release->thread->isRunning();
    if (!release->worker && !release->client && !release->channel
        && !release->ioThread && !release->thread) {
        syncConnectionWidget(SessionConnectionState::Disconnected);
        emit stackReleased();
        maybeRunDeferredAction();
        return;
    }

    pendingReleases_.push_back(release);
    syncConnectionWidget(SessionConnectionState::Disconnecting);

    // Bounded async shutdown: if completion does not arrive in time, detach safely
    // and surface a controlled failure instead of forcibly terminating threads.
    release->timeoutTimer = new QTimer(this);
    release->timeoutTimer->setSingleShot(true);
    QObject::connect(release->timeoutTimer, &QTimer::timeout, this, [this, release]() {
        handleReleaseTimeout(release);
    });
    release->timeoutTimer->start(5000);

    if (release->thread) {
        QObject::connect(release->thread.get(), &QThread::finished, this,
                         [this, release]() { onReleaseThreadFinished(release, false); },
                         Qt::QueuedConnection);
    }
    if (release->ioThread) {
        QObject::connect(release->ioThread.get(), &QThread::finished, this,
                         [this, release]() { onReleaseThreadFinished(release, true); },
                         Qt::QueuedConnection);
    }

    if (release->thread && release->thread->isRunning()) {
        release->thread->requestInterruption();
    }
    if (release->ioThread && release->ioThread->isRunning()) {
        release->ioThread->requestInterruption();
    }

    if (release->worker) {
        QObject::connect(release->worker.get(), &::modbus::dispatch::ModbusWorker::stopped,
                         this,
                         [this, release]() { onReleaseWorkerStopped(release); },
                         Qt::QueuedConnection);
        release->worker->stop();
    }
    if (release->thread && release->thread->isRunning()) {
        release->thread->quit();
    }
    if (release->ioThread && release->ioThread->isRunning()) {
        release->ioThread->quit();
    }
    tryCompletePendingRelease(release);
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
    return connectionState_ == SessionConnectionState::Connected;
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

void ModbusSessionPresenter::setConnectionWidget(QWidget* widget) {
    assertGuiThread("setConnectionWidget must be called on the GUI thread");
    connectionWidget_ = widget;
    syncConnectionWidget(connectionState_);
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

void ModbusSessionPresenter::applyConnectionState(SessionConnectionState state) {
    assertGuiThread("applyConnectionState must run on the GUI thread");
    connectionState_ = state;
    syncConnectionWidget(state);
}

void ModbusSessionPresenter::syncConnectionWidget(SessionConnectionState state) {
    assertGuiThread("syncConnectionWidget must run on the GUI thread");
    if (!connectionWidget_) {
        return;
    }

    switch (state) {
    case SessionConnectionState::Disconnected:
        QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection, Q_ARG(bool, false));
        return;
    case SessionConnectionState::Connected:
        QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection, Q_ARG(bool, true));
        return;
    case SessionConnectionState::Connecting:
        setConnectionWidgetDisplayStateImpl(connectionWidget_, mode_, ui::widgets::BaseConnectionWidget::DisplayState::Connecting);
        return;
    case SessionConnectionState::TransportConnected:
        setConnectionWidgetDisplayStateImpl(connectionWidget_, mode_, ui::widgets::BaseConnectionWidget::DisplayState::TransportConnected);
        return;
    case SessionConnectionState::Disconnecting:
        setConnectionWidgetDisplayStateImpl(connectionWidget_, mode_, ui::widgets::BaseConnectionWidget::DisplayState::Disconnecting);
        return;
    }
}

void ModbusSessionPresenter::onReleaseWorkerStopped(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    assertGuiThread("onReleaseWorkerStopped must run on the GUI thread");
    if (!pending) {
        return;
    }
    pending->workerStopped = true;
    if (pending->thread && pending->thread->isRunning()) {
        pending->thread->quit();
    }
    if (pending->ioThread && pending->ioThread->isRunning()) {
        pending->ioThread->quit();
    }
    tryCompletePendingRelease(pending);
}

void ModbusSessionPresenter::onReleaseThreadFinished(
    const std::shared_ptr<PendingReleaseContext>& pending, bool ioThread) {
    assertGuiThread("onReleaseThreadFinished must run on the GUI thread");
    if (!pending) {
        return;
    }
    if (ioThread) {
        pending->ioThreadFinished = true;
    } else {
        pending->threadFinished = true;
    }
    tryCompletePendingRelease(pending);
}

void ModbusSessionPresenter::tryCompletePendingRelease(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    assertGuiThread("tryCompletePendingRelease must run on the GUI thread");
    if (!pending) {
        return;
    }
    const bool workerDone = pending->workerStopped;
    const bool ioDone = pending->ioThreadFinished || !pending->ioThread || !pending->ioThread->isRunning();
    const bool workerThreadDone = pending->threadFinished || !pending->thread || !pending->thread->isRunning();
    if (!workerDone || !ioDone || !workerThreadDone) {
        return;
    }
    finalizePendingRelease(pending);
}

void ModbusSessionPresenter::handleReleaseTimeout(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    assertGuiThread("handleReleaseTimeout must run on the GUI thread");
    if (!pending) {
        return;
    }
    const auto it = std::find(pendingReleases_.begin(), pendingReleases_.end(), pending);
    if (it == pendingReleases_.end()) {
        return;
    }
    if (!pending->completionLogged) {
        pending->completionLogged = true;
        spdlog::error("ModbusSessionPresenter: shutdown timed out; finalizing without terminate()");
        if (trafficLogController_) {
            trafficLogController_->logError(pending->timeoutMessage);
        }
        emit sessionDisconnected(pending->timeoutMessage);
    }
    if (pending->thread && pending->thread->isRunning()) {
        pending->thread->quit();
    }
    if (pending->ioThread && pending->ioThread->isRunning()) {
        pending->ioThread->quit();
    }
    finalizePendingRelease(pending);
}

void ModbusSessionPresenter::maybeRunDeferredAction() {
    assertGuiThread("maybeRunDeferredAction must run on the GUI thread");
    if (pendingReleases_.empty() && deferredAction_) {
        auto action = std::move(deferredAction_);
        deferredAction_ = nullptr;
        QMetaObject::invokeMethod(this, [action = std::move(action)]() mutable {
            if (action) {
                action();
            }
        }, Qt::QueuedConnection);
    }
}

void ModbusSessionPresenter::finalizePendingRelease(const std::shared_ptr<PendingReleaseContext>& pending) {
    assertGuiThread("finalizePendingRelease must run on the GUI thread");
    if (!pending) {
        if (!worker_ && !channel_ && !client_) {
            syncConnectionWidget(SessionConnectionState::Disconnected);
        }
        emit stackReleased();
        maybeRunDeferredAction();
        return;
    }

    const auto it = std::find(pendingReleases_.begin(), pendingReleases_.end(), pending);
    if (it == pendingReleases_.end()) {
        return;
    }
    pendingReleases_.erase(it);

    if (pending->timeoutTimer) {
        pending->timeoutTimer->stop();
        pending->timeoutTimer->deleteLater();
        pending->timeoutTimer = nullptr;
    }

    if (pending->thread && pending->thread->isRunning()) {
        pending->thread->quit();
    }
    if (pending->ioThread && pending->ioThread->isRunning()) {
        pending->ioThread->quit();
    }

    pending->worker.reset();
    pending->client.reset();
    pending->channel.reset();
    pending->ioThread.reset();
    pending->thread.reset();

    if (!worker_ && !channel_ && !client_ && connectionState_ != SessionConnectionState::Connecting) {
        syncConnectionWidget(SessionConnectionState::Disconnected);
    }

    emit stackReleased();
    maybeRunDeferredAction();
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
    ::modbus::factory::ModbusFactory factory;
    auto stack = factory.createStack(config);
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
    ioThread_ = std::move(stack.ioThread);
    workerThread_ = std::move(stack.thread);
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
    const bool hadLiveTransport = connectionState_ != SessionConnectionState::Disconnected;
    const bool isTcp = (mode_ == SessionMode::Tcp);

    switch (state) {
    case io::ChannelState::Opening:
        if (connectionState_ != SessionConnectionState::Connected) {
            syncConnectionWidget(SessionConnectionState::Connecting);
        }
        return;
    case io::ChannelState::Open: {
        const bool wasNotConnected = (connectionState_ != SessionConnectionState::Connected);
        connectionState_ = SessionConnectionState::TransportConnected;
        if (wasNotConnected) {
            syncConnectionWidget(connectionState_);
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
        const bool wasConnected = (connectionState_ == SessionConnectionState::Connected);
        connectionState_ = SessionConnectionState::Disconnected;
        if (wasConnected || hadLiveTransport) {
            const bool shouldShowDisconnectAlert = isTcp && wasConnected && !suppressDisconnectAlert_;
            syncConnectionWidget(connectionState_);
            if (controlWidget_) {
                controlWidget_->setPollingEnabled(false);
            }
            if (isTcp && trafficLogController_) {
                trafficLogController_->logConnectionInfo(tr("Disconnected"));
            }
            if (shouldShowDisconnectAlert) {
                ui::common::ConnectionAlert::showDisconnected(qApp->activeWindow());
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
        applyConnectionState(SessionConnectionState::Disconnected);
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

    suppressDisconnectAlert_ = false;
    applyConnectionState(SessionConnectionState::Connected);
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
                                                response.retryCount,
                                                response.error);
        }
    }

    switch (response.kind) {
    case ::modbus::session::ModbusResponseKind::NoResponseExpected:
        if (trafficLogController_) {
            trafficLogController_->logBroadcastWriteSuccess(response.retryCount);
        }
        break;
    case ::modbus::session::ModbusResponseKind::Success:
        if (controlWidget_) {
            controlWidget_->recordRx(response.rttMs);
        }

        if (kind == RequestKind::Read && trafficLogController_) {
            trafficLogController_->logReadSuccess(response.retryCount);
        } else if (kind == RequestKind::Write && trafficLogController_) {
            trafficLogController_->logWriteSuccess(response.retryCount);
        }
        break;
    case ::modbus::session::ModbusResponseKind::Error:
        if (controlWidget_) {
            controlWidget_->recordError();
        }

        if (kind != RequestKind::Poll && trafficLogController_) {
            trafficLogController_->logRequestError(response.error, response.retryCount);
        }
        break;
    }

    emit requestFinished(requestId, response);
}

} // namespace ui::application::modbus
