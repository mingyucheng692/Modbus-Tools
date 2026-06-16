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
#include <QApplication>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace ui::application::modbus {

ModbusSessionPresenter::ModbusSessionPresenter(SessionMode mode, QObject* parent)
    : QObject(parent),
      mode_(mode),
      timeoutMs_(app::constants::Values::Modbus::kDefaultTimeoutMs),
      retries_(0),
      retryIntervalMs_(app::constants::Values::Modbus::kDefaultRetryIntervalMs) {
}

ModbusSessionPresenter::~ModbusSessionPresenter() {
    releaseStack();
}

void ModbusSessionPresenter::connectTcp(const QString& ip, int port,
                                         const ::modbus::base::ModbusConfig& config) {
    Q_ASSERT(mode_ == SessionMode::Tcp);
    spdlog::info("ModbusSessionPresenter[TCP]: Connect requested to {}:{}", ip.toStdString(), port);
    releaseStack();
    suppressDisconnectAlert_ = false;
    connectionState_ = SessionConnectionState::Connecting;
    const quint64 generation = connectionGeneration_;
    setConnectionWidgetConnecting();

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    }

    initStack(config);
    if (!worker_ || !channel_) {
        connectionState_ = SessionConnectionState::Disconnected;
        setConnectionWidgetDisconnected();
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

    worker_->start();
    worker_->requestConnect();
}

void ModbusSessionPresenter::connectRtu(const io::SerialConfig& serialConfig,
                                         const ::modbus::base::ModbusConfig& modbusConfig) {
    Q_ASSERT(mode_ == SessionMode::Rtu);
    spdlog::info("ModbusSessionPresenter[RTU]: Connect requested to {}", serialConfig.portName.toStdString());

    releaseStack();
    connectionState_ = SessionConnectionState::Connecting;
    setConnectionWidgetConnecting();

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Opening %1...").arg(serialConfig.portName));
    }
    const quint64 generation = connectionGeneration_;

    initStack(modbusConfig);
    if (!worker_ || !channel_) {
        connectionState_ = SessionConnectionState::Disconnected;
        setConnectionWidgetDisconnected();
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

    worker_->start();
    worker_->requestConnect();
}

void ModbusSessionPresenter::requestDisconnect() {
    spdlog::info("ModbusSessionPresenter[{}]: Disconnect requested",
                 mode_ == SessionMode::Tcp ? "TCP" : "RTU");
    suppressDisconnectAlert_ = true;
    connectionState_ = SessionConnectionState::Disconnecting;
    setConnectionWidgetDisconnecting();
    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Disconnecting..."));
    }
    releaseStack();
}

void ModbusSessionPresenter::releaseStack() {
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
    release->thread = std::move(workerThread_);
    if (!release->worker && !release->client && !release->channel && !release->thread) {
        setConnectionWidgetDisconnected();
        emit stackReleased();
        return;
    }

    pendingReleases_.push_back(release);
    setConnectionWidgetDisconnecting();

    if (release->thread && release->thread->isRunning()) {
        release->thread->requestInterruption();
    }

    if (release->worker) {
        QObject::connect(release->worker.get(), &::modbus::dispatch::ModbusWorker::stopped,
                         this,
                         [this, release]() { finalizePendingRelease(release); },
                         Qt::QueuedConnection);
        release->worker->stop();
        return;
    }

    if (release->thread && release->thread->isRunning()) {
        release->thread->quit();
    }
    finalizePendingRelease(release);
}

void ModbusSessionPresenter::updateSettings(const ModbusTimingParams& params) {
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
    if (worker_) {
        worker_->submit(pdu, slaveId, requestId);
    }
}

void ModbusSessionPresenter::sendRaw(const QByteArray& data) {
    if (worker_) {
        worker_->sendRaw(data);
    }
}

void ModbusSessionPresenter::setTrafficLogController(TrafficLogController* controller) {
    trafficLogController_ = controller;
}

void ModbusSessionPresenter::setPollingController(PollingController* controller) {
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
    pollingController_->setSessionConnected(connectionState_ == SessionConnectionState::Connected);
}

void ModbusSessionPresenter::setRequestService(RequestSubmissionService* service) {
    requestService_ = service;
}

void ModbusSessionPresenter::setConnectionWidget(QWidget* widget) {
    connectionWidget_ = widget;
}

void ModbusSessionPresenter::setControlWidget(ui::widgets::ControlWidget* widget) {
    controlWidget_ = widget;
}

namespace {
void setConnectionWidgetDisplayStateImpl(QWidget* connectionWidget,
                                         SessionMode mode,
                                         ui::widgets::BaseConnectionWidget::DisplayState displayState) {
    if (!connectionWidget) return;

    if (mode == SessionMode::Tcp) {
        if (auto* connWidget = qobject_cast<ui::widgets::BaseConnectionWidget*>(connectionWidget)) {
            QMetaObject::invokeMethod(connWidget,
                [connWidget, displayState]() { connWidget->setDisplayState(displayState); },
                Qt::QueuedConnection);
        }
        return;
    }

    if (auto* serialWidget = qobject_cast<ui::widgets::SerialConnectionWidget*>(connectionWidget)) {
        QMetaObject::invokeMethod(serialWidget,
            [serialWidget, displayState]() { serialWidget->setDisplayState(displayState); },
            Qt::QueuedConnection);
    }
}
} // namespace

void ModbusSessionPresenter::setConnectionWidgetConnecting() {
    setConnectionWidgetDisplayStateImpl(connectionWidget_, mode_, ui::widgets::BaseConnectionWidget::DisplayState::Connecting);
}

void ModbusSessionPresenter::setConnectionWidgetTransportConnected() {
    setConnectionWidgetDisplayStateImpl(connectionWidget_, mode_, ui::widgets::BaseConnectionWidget::DisplayState::TransportConnected);
}

void ModbusSessionPresenter::setConnectionWidgetConnected() {
    if (!connectionWidget_) {
        return;
    }

    QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection, Q_ARG(bool, true));
}

void ModbusSessionPresenter::setConnectionWidgetDisconnecting() {
    setConnectionWidgetDisplayStateImpl(connectionWidget_, mode_, ui::widgets::BaseConnectionWidget::DisplayState::Disconnecting);
}

void ModbusSessionPresenter::setConnectionWidgetDisconnected() {
    if (!connectionWidget_) {
        return;
    }

    QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection, Q_ARG(bool, false));
}

void ModbusSessionPresenter::finalizePendingRelease(const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        if (!worker_ && !channel_ && !client_) {
            setConnectionWidgetDisconnected();
        }
        emit stackReleased();
        return;
    }

    const auto it = std::find(pendingReleases_.begin(), pendingReleases_.end(), pending);
    if (it == pendingReleases_.end()) {
        return;
    }
    pendingReleases_.erase(it);

    if (pending->thread && pending->thread->isRunning()) {
        pending->thread->quit();
    }

    pending->worker.reset();
    pending->client.reset();
    pending->channel.reset();
    pending->thread.reset();

    if (!worker_ && !channel_ && !client_ && connectionState_ != SessionConnectionState::Connecting) {
        setConnectionWidgetDisconnected();
    }

    emit stackReleased();
}

void ModbusSessionPresenter::setLinked(bool linked) {
    linked_ = linked;
    if (controlWidget_) {
        controlWidget_->setLinked(linked);
    }
}

bool ModbusSessionPresenter::isLinked() const {
    return linked_;
}

void ModbusSessionPresenter::initStack(const ::modbus::base::ModbusConfig& config) {
    ::modbus::factory::ModbusFactory factory;
    auto stack = factory.createStack(config);
    if (!stack.worker || !stack.thread) {
        if (trafficLogController_) {
            trafficLogController_->logError(tr("Failed to create Modbus stack"));
        }
        return;
    }

    currentConfig_ = config;
    channel_ = std::move(stack.channel);
    client_ = std::move(stack.client);
    worker_ = std::move(stack.worker);
    workerThread_ = std::move(stack.thread);
}

void ModbusSessionPresenter::setupChannelMonitor(quint64 generation) {
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
    Q_UNUSED(generation);
    const bool hadLiveTransport = connectionState_ != SessionConnectionState::Disconnected;
    const bool isTcp = (mode_ == SessionMode::Tcp);

    switch (state) {
    case io::ChannelState::Opening:
        if (connectionState_ != SessionConnectionState::Connected) {
            setConnectionWidgetConnecting();
        }
        return;
    case io::ChannelState::Open: {
        const bool wasNotConnected = (connectionState_ != SessionConnectionState::Connected);
        connectionState_ = SessionConnectionState::TransportConnected;
        if (wasNotConnected) {
            setConnectionWidgetTransportConnected();
            if (isTcp && trafficLogController_) {
                trafficLogController_->logConnectionInfo(tr("Transport connected, validating session..."));
            }
        }
        return;
    }
    case io::ChannelState::Closing:
        setConnectionWidgetDisconnecting();
        return;
    case io::ChannelState::Closed:
    case io::ChannelState::Error: {
        const bool wasConnected = (connectionState_ == SessionConnectionState::Connected);
        connectionState_ = SessionConnectionState::Disconnected;
        if (wasConnected || hadLiveTransport) {
            const bool shouldShowDisconnectAlert = isTcp && wasConnected && !suppressDisconnectAlert_;
            setConnectionWidgetDisconnected();
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
    if (generation != connectionGeneration_) return;

    if (!ok) {
        connectionState_ = SessionConnectionState::Disconnected;
        setConnectionWidgetDisconnected();
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
    connectionState_ = SessionConnectionState::Connected;
    setConnectionWidgetConnected();
    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Connected"));
    }
    emit sessionConnected();
    emit connectFinished(true, error);
}

void ModbusSessionPresenter::handleRequestFinished(int requestId,
                                                     const ::modbus::session::ModbusResponse& response,
                                                     quint64 generation) {
    Q_ASSERT_X(QThread::currentThread() == thread(),
               "ModbusSessionPresenter",
               "handleRequestFinished must be called on the main thread");

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
            pollingController_->handleResponse(response.isSuccess,
                                                response.rttMs,
                                                response.retryCount,
                                                response.error);
        }
    }

    if (response.isSuccess && response.noResponseExpected) {
        if (trafficLogController_) {
            trafficLogController_->logBroadcastWriteSuccess(response.retryCount);
        }
    } else if (response.isSuccess) {
        if (controlWidget_) {
            controlWidget_->recordRx(response.rttMs);
        }

        if (kind == RequestKind::Read && trafficLogController_) {
            trafficLogController_->logReadSuccess(response.retryCount);
        } else if (kind == RequestKind::Write && trafficLogController_) {
            trafficLogController_->logWriteSuccess(response.retryCount);
        }
    } else {
        if (controlWidget_) {
            controlWidget_->recordError();
        }

        if (kind != RequestKind::Poll && trafficLogController_) {
            trafficLogController_->logRequestError(response.error, response.retryCount);
        }
    }

    emit requestFinished(requestId, response);
}

} // namespace ui::application::modbus
