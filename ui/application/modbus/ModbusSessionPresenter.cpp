#include "ModbusSessionPresenter.h"
#include "RequestSubmissionService.h"
#include "PollingController.h"
#include "TrafficLogController.h"
#include "../../widgets/ControlWidget.h"
#include "../../common/ConnectionAlert.h"
#include "../../common/TcpConnectionStateCoordinator.h"
#include "modbus/factory/ModbusFactory.h"
#include <QEventLoop>
#include <QMetaObject>
#include <QPointer>
#include <QApplication>
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
    const quint64 generation = connectionGeneration_;

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    }

    initStack(config);

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

    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Opening %1...").arg(serialConfig.portName));
    }

    releaseStack();
    const quint64 generation = connectionGeneration_;

    initStack(modbusConfig);

    setupChannelMonitor(generation);
    setupChannelStateHandler(generation);
    setupWorkerSignals(generation);

    worker_->start();
    worker_->requestConnect();
}

void ModbusSessionPresenter::requestDisconnect() {
    spdlog::info("ModbusSessionPresenter[{}]: Disconnect requested",
                 mode_ == SessionMode::Tcp ? "TCP" : "RTU");
    if (mode_ == SessionMode::Tcp) {
        suppressDisconnectAlert_ = true;
    }
    sessionConnected_ = false;
    if (connectionWidget_) {
        QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                  Q_ARG(bool, false));
    }
    if (trafficLogController_) {
        trafficLogController_->logConnectionInfo(tr("Disconnected"));
    }
    releaseStack();
}

void ModbusSessionPresenter::releaseStack() {
    if (pollingController_) pollingController_->reset();
    ++connectionGeneration_;
    suppressDisconnectAlert_ = true;
    sessionConnected_ = false;
    if (connectionWidget_) {
        QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                  Q_ARG(bool, false));
    }
    const bool wasLinked = linked_;
    linked_ = false;
    if (controlWidget_) {
        controlWidget_->setLinked(false);
        controlWidget_->setPollingEnabled(false);
    }
    if (wasLinked) {
        emit linkageSourceDisconnected();
    }

    auto channel = std::move(channel_);
    auto client = std::move(client_);
    auto worker = std::move(worker_);
    auto thread = std::move(workerThread_);
    if (!worker && !client && !channel && !thread) {
        emit stackReleased();
        return;
    }

    if (worker) {
        QEventLoop wait;
        QObject::connect(worker.get(), &::modbus::dispatch::ModbusWorker::stopped,
                         &wait, &QEventLoop::quit);
        worker->stop();
        wait.exec();
    } else if (thread && thread->isRunning()) {
        thread->requestInterruption();
        thread->quit();
    }
    worker.reset();
    client.reset();
    if (thread && thread->isRunning()) {
        thread->requestInterruption();
        thread->quit();
        thread->wait(2000);
    }
    channel.reset();
    thread.reset();

    emit stackReleased();
}

void ModbusSessionPresenter::updateSettings(int timeoutMs, int retries, int retryIntervalMs) {
    timeoutMs_ = timeoutMs;
    retries_ = retries;
    retryIntervalMs_ = retryIntervalMs;
    if (worker_) {
        currentConfig_.timeoutMs = timeoutMs_;
        currentConfig_.retries = retries_;
        currentConfig_.retryIntervalMs = retryIntervalMs_;
        worker_->updateConfig(currentConfig_);
    }
}

bool ModbusSessionPresenter::isSessionConnected() const {
    return sessionConnected_;
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
    pollingController_ = controller;
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
            if (guard->mode_ == SessionMode::Tcp) {
                guard->handleTcpStateTransition(state, generation);
            } else {
                guard->handleRtuStateTransition(state, generation);
            }
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

void ModbusSessionPresenter::handleTcpStateTransition(io::ChannelState state,
                                                       quint64 generation) {
    const bool wasConnected = sessionConnected_;
    const auto transition = ui::common::TcpConnectionStateCoordinator::transition(
        state, wasConnected, suppressDisconnectAlert_);

    if (transition.setConnected) {
        sessionConnected_ = true;
        if (connectionWidget_) {
            QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                      Q_ARG(bool, true));
        }
        if (trafficLogController_) {
            trafficLogController_->logConnectionInfo(tr("Connected"));
        }
        emit sessionConnected();
    } else if (state == io::ChannelState::Open) {
        sessionConnected_ = true;
        if (connectionWidget_) {
            QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                      Q_ARG(bool, true));
        }
    }
    if (transition.clearSuppressDisconnectAlert) {
        suppressDisconnectAlert_ = false;
    }
    if (transition.setConnected) {
        return;
    }
    if (transition.setDisconnected) {
        sessionConnected_ = false;
        if (connectionWidget_) {
            QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                      Q_ARG(bool, false));
        }
        if (controlWidget_) {
            controlWidget_->setPollingEnabled(false);
        }
        if (trafficLogController_) {
            trafficLogController_->logConnectionInfo(tr("Disconnected"));
        }
        if (transition.showDisconnectAlert) {
            ui::common::ConnectionAlert::showDisconnected(qApp->activeWindow());
        }
        emit sessionDisconnected(QString());
    }
}

void ModbusSessionPresenter::handleRtuStateTransition(io::ChannelState state,
                                                       quint64 generation) {
    const bool connected = (state == io::ChannelState::Open);
    sessionConnected_ = connected;
    if (connectionWidget_) {
        QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                  Q_ARG(bool, connected));
    }
    if (!connected) {
        if (controlWidget_) {
            controlWidget_->setPollingEnabled(false);
        }
        emit sessionDisconnected(QString());
    } else {
        emit sessionConnected();
    }
}

void ModbusSessionPresenter::handleConnectFinished(bool ok, const QString& error,
                                                    quint64 generation) {
    if (generation != connectionGeneration_) return;

    if (mode_ == SessionMode::Tcp) {
        const bool wasConnected = sessionConnected_;
        sessionConnected_ = ok;
        if (!ok) {
            if (pollingController_) pollingController_->stopPoll();
        }
        if (ok) {
            suppressDisconnectAlert_ = false;
            if (connectionWidget_) {
                QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                          Q_ARG(bool, true));
            }
            if (!wasConnected && trafficLogController_) {
                trafficLogController_->logConnectionInfo(tr("Connected"));
            }
        } else {
            if (connectionWidget_) {
                QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                          Q_ARG(bool, false));
            }
            if (trafficLogController_) {
                trafficLogController_->logConnectionInfo(tr("Connection failed: %1").arg(error));
            }
        }
    } else {
        sessionConnected_ = ok;
        if (!ok) {
            if (pollingController_) pollingController_->stopPoll();
        }
        if (ok) {
            if (connectionWidget_) {
                QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                          Q_ARG(bool, true));
            }
            if (trafficLogController_) {
                trafficLogController_->logConnectionInfo(tr("Connected"));
            }
        } else {
            if (connectionWidget_) {
                QMetaObject::invokeMethod(connectionWidget_, "setConnected", Qt::QueuedConnection,
                                          Q_ARG(bool, false));
            }
            if (trafficLogController_) {
                trafficLogController_->logConnectionInfo(tr("Connection failed: %1").arg(error));
            }
        }
    }

    emit connectFinished(ok, error);
}

void ModbusSessionPresenter::handleRequestFinished(int requestId,
                                                     const ::modbus::session::ModbusResponse& response,
                                                     quint64 generation) {
    Q_ASSERT_X(QThread::currentThread() == thread(),
               "ModbusSessionPresenter",
               "handleRequestFinished must be called on the main thread");

    if (generation != connectionGeneration_) return;
    if (!requestService_) return;

    auto trackingInfo = requestService_->lookupAndRemove(requestId);
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
