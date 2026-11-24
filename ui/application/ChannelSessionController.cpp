// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ChannelSessionController.h"
#include "infra/io/ChannelOperationWorker.h"
#include "infra/io/ServerChannelWorker.h"
#include "core/common/ThreadGuard.h"
#include <QThread>
#include <QTimer>
#include <QMetaObject>

namespace ui::application {

ChannelSessionController::ChannelSessionController(QObject* parent)
    : QObject(parent)
    , autoSendTimer_(new QTimer(this))
{
    connect(autoSendTimer_, &QTimer::timeout, this, &ChannelSessionController::onAutoSendTimeout);
    updateSnapshot(false);
}

ChannelSessionController::~ChannelSessionController() noexcept {
    if (autoSendTimer_) {
        autoSendTimer_->stop();
    }

    if (operationWorker_) {
        QObject::disconnect(operationWorker_.get(), nullptr, this, nullptr);
    }
    if (serverWorker_) {
        QObject::disconnect(serverWorker_.get(), nullptr, this, nullptr);
    }

    if (operationThread_) {
        core::common::ThreadGuard::stopThread(operationThread_.get(), 1000);
    }
    operationWorker_.reset();
    operationThread_.reset();

    if (serverThread_) {
        core::common::ThreadGuard::stopThread(serverThread_.get(), 1000);
    }
    serverWorker_.reset();
    serverThread_.reset();
}

io::ChannelOperationWorker* ChannelSessionController::ensureOperationWorker() {
    if (!operationWorker_) {
        operationThread_ = std::make_shared<QThread>();
        operationWorker_ = std::make_shared<io::ChannelOperationWorker>();
        operationWorker_->moveToThread(operationThread_.get());

        connect(operationWorker_.get(), &io::ChannelOperationWorker::stateChanged,
                this, &ChannelSessionController::onWorkerStateChanged);
        connect(operationWorker_.get(), &io::ChannelOperationWorker::channelErrorOccurred,
                this, &ChannelSessionController::errorOccurred);
        connect(operationWorker_.get(), &io::ChannelOperationWorker::monitor,
                this, &ChannelSessionController::trafficOccurred);

        operationThread_->start();
    }
    return operationWorker_.get();
}

io::ServerChannelWorker* ChannelSessionController::ensureServerWorker() {
    if (!serverWorker_) {
        serverThread_ = std::make_shared<QThread>();
        serverWorker_ = std::make_shared<io::ServerChannelWorker>();
        serverWorker_->moveToThread(serverThread_.get());

        connect(serverWorker_.get(), &io::ServerChannelWorker::stateChanged,
                this, &ChannelSessionController::onWorkerStateChanged);
        connect(serverWorker_.get(), &io::ServerChannelWorker::channelErrorOccurred,
                this, &ChannelSessionController::errorOccurred);
        connect(serverWorker_.get(), &io::ServerChannelWorker::monitorWithClient,
                this, &ChannelSessionController::clientTrafficOccurred);
        connect(serverWorker_.get(), &io::ServerChannelWorker::clientConnected,
                this, &ChannelSessionController::clientConnected);
        connect(serverWorker_.get(), &io::ServerChannelWorker::clientDisconnected,
                this, &ChannelSessionController::clientDisconnected);

        serverThread_->start();
    }
    return serverWorker_.get();
}

void ChannelSessionController::openTcp(const QString& host, int port) {
    auto* worker = ensureOperationWorker();
    const quint64 gen = ++generation_;
    activeRole_ = ChannelRole::TcpClient;
    transitionTo(io::ChannelState::Opening);

    QMetaObject::invokeMethod(worker, "openTcp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, host),
                              Q_ARG(int, port),
                              Q_ARG(quint64, gen));
}

void ChannelSessionController::openUdp(const QString& localIp, int localPort, const QString& remoteIp, int remotePort) {
    auto* worker = ensureOperationWorker();
    const quint64 gen = ++generation_;
    activeRole_ = ChannelRole::Udp;
    transitionTo(io::ChannelState::Opening);

    QMetaObject::invokeMethod(worker, "openUdp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, localIp),
                              Q_ARG(int, localPort),
                              Q_ARG(QString, remoteIp),
                              Q_ARG(int, remotePort),
                              Q_ARG(quint64, gen));
}

void ChannelSessionController::openSerial(const io::SerialConfig& config) {
    auto* worker = ensureOperationWorker();
    const quint64 gen = ++generation_;
    activeRole_ = ChannelRole::Serial;
    transitionTo(io::ChannelState::Opening);

    QMetaObject::invokeMethod(worker, "openSerial",
                              Qt::QueuedConnection,
                              Q_ARG(io::SerialConfig, config),
                              Q_ARG(quint64, gen));
}

void ChannelSessionController::openTcpServer(const QString& listenIp, int port, int maxClients) {
    auto* worker = ensureServerWorker();
    const quint64 gen = ++generation_;
    activeRole_ = ChannelRole::TcpServer;
    transitionTo(io::ChannelState::Opening);

    QMetaObject::invokeMethod(worker, "openTcpServer",
                              Qt::QueuedConnection,
                              Q_ARG(QString, listenIp),
                              Q_ARG(int, port),
                              Q_ARG(int, maxClients),
                              Q_ARG(quint64, gen));
}

void ChannelSessionController::closeChannel() {
    if (autoSendTimer_ && autoSendTimer_->isActive()) {
        autoSendTimer_->stop();
        emit autoSendStateChanged(false);
    }

    if (state_ == io::ChannelState::Opening) {
        // Discard any in-flight connection response from an earlier generation
        ++generation_;
        transitionTo(io::ChannelState::Closing);
        if (activeRole_ == ChannelRole::TcpServer && serverWorker_) {
            QMetaObject::invokeMethod(serverWorker_.get(), "stopServer", Qt::QueuedConnection);
        } else if (operationWorker_) {
            QMetaObject::invokeMethod(operationWorker_.get(), "close", Qt::QueuedConnection);
        }
        return;
    }

    if (state_ == io::ChannelState::Open) {
        transitionTo(io::ChannelState::Closing);
        if (activeRole_ == ChannelRole::TcpServer && serverWorker_) {
            QMetaObject::invokeMethod(serverWorker_.get(), "stopServer", Qt::QueuedConnection);
        } else if (operationWorker_) {
            QMetaObject::invokeMethod(operationWorker_.get(), "close", Qt::QueuedConnection);
        }
        return;
    }

    if (state_ == io::ChannelState::Error) {
        transitionTo(io::ChannelState::Closed);
    }
}

void ChannelSessionController::write(const QByteArray& data) {
    if (!currentSnapshot_.sendControlsEnabled || data.isEmpty()) {
        return;
    }
    if (activeRole_ == ChannelRole::TcpServer) {
        return; // Server writing requires explicit target(s); handled via writeToClient(s)
    }
    if (operationWorker_) {
        QMetaObject::invokeMethod(operationWorker_.get(), "write",
                                  Qt::QueuedConnection,
                                  Q_ARG(QByteArray, data));
    }
}

void ChannelSessionController::writeToClient(int clientId, const QByteArray& data) {
    if (!currentSnapshot_.sendControlsEnabled || data.isEmpty()) {
        return;
    }
    if (serverWorker_) {
        QMetaObject::invokeMethod(serverWorker_.get(), "writeToClient",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, clientId),
                                  Q_ARG(QByteArray, data));
    }
}

void ChannelSessionController::writeToClients(const QList<int>& clientIds, const QByteArray& data) {
    if (!currentSnapshot_.sendControlsEnabled || data.isEmpty() || clientIds.isEmpty()) {
        return;
    }
    for (int clientId : clientIds) {
        writeToClient(clientId, data);
    }
}

void ChannelSessionController::setServerTargetState(bool hasTargetClients, bool broadcastMode) {
    if (serverHasTargets_ == hasTargetClients && serverBroadcastMode_ == broadcastMode) {
        return;
    }
    serverHasTargets_ = hasTargetClients;
    serverBroadcastMode_ = broadcastMode;
    updateSnapshot(false);

    if (!currentSnapshot_.sendControlsEnabled && autoSendTimer_ && autoSendTimer_->isActive()) {
        autoSendTimer_->stop();
        emit autoSendStateChanged(false);
    }
}

void ChannelSessionController::setRole(ChannelRole role) {
    if (activeRole_ != role) {
        activeRole_ = role;
        updateSnapshot(false);
    }
}

void ChannelSessionController::setAutoSend(bool enabled, int intervalMs,
                                          std::function<QByteArray()> dataSupplier,
                                          std::function<void(const QByteArray&)> sendAction) {
    if (dataSupplier) {
        dataSupplier_ = std::move(dataSupplier);
    }
    if (sendAction) {
        sendAction_ = std::move(sendAction);
    }
    if (!enabled) {
        if (autoSendTimer_ && autoSendTimer_->isActive()) {
            autoSendTimer_->stop();
            emit autoSendStateChanged(false);
        }
        return;
    }

    if (!currentSnapshot_.sendControlsEnabled) {
        if (autoSendTimer_ && autoSendTimer_->isActive()) {
            autoSendTimer_->stop();
        }
        emit autoSendStateChanged(false);
        return;
    }

    if (autoSendTimer_) {
        autoSendTimer_->setInterval(intervalMs > 0 ? intervalMs : 1000);
        if (!autoSendTimer_->isActive()) {
            autoSendTimer_->start();
            emit autoSendStateChanged(true);
        }
    }
}

void ChannelSessionController::setAutoSendInterval(int intervalMs) {
    if (autoSendTimer_ && intervalMs > 0) {
        autoSendTimer_->setInterval(intervalMs);
    }
}

bool ChannelSessionController::isAutoSendActive() const {
    return autoSendTimer_ && autoSendTimer_->isActive();
}

void ChannelSessionController::onAutoSendTimeout() {
    if (!currentSnapshot_.sendControlsEnabled) {
        if (autoSendTimer_ && autoSendTimer_->isActive()) {
            autoSendTimer_->stop();
            emit autoSendStateChanged(false);
        }
        return;
    }
    if (dataSupplier_) {
        const QByteArray payload = dataSupplier_();
        if (!payload.isEmpty()) {
            if (sendAction_) {
                sendAction_(payload);
            } else {
                write(payload);
            }
        }
    }
}

void ChannelSessionController::setDtr(bool set) {
    if (activeRole_ == ChannelRole::Serial && operationWorker_) {
        QMetaObject::invokeMethod(operationWorker_.get(), "setDtr",
                                  Qt::QueuedConnection,
                                  Q_ARG(bool, set));
    }
}

void ChannelSessionController::setRts(bool set) {
    if (activeRole_ == ChannelRole::Serial && operationWorker_) {
        QMetaObject::invokeMethod(operationWorker_.get(), "setRts",
                                  Qt::QueuedConnection,
                                  Q_ARG(bool, set));
    }
}

void ChannelSessionController::disconnectClients(const QList<int>& clientIds) {
    if (activeRole_ == ChannelRole::TcpServer && serverWorker_) {
        for (int clientId : clientIds) {
            QMetaObject::invokeMethod(serverWorker_.get(), "closeClient",
                                      Qt::QueuedConnection,
                                      Q_ARG(int, clientId));
        }
    }
}

void ChannelSessionController::disconnectAllClients() {
    if (activeRole_ == ChannelRole::TcpServer && serverWorker_) {
        QMetaObject::invokeMethod(serverWorker_.get(), "disconnectAllClients", Qt::QueuedConnection);
    }
}

void ChannelSessionController::onWorkerStateChanged(io::ChannelState nextState, quint64 generation) {
    if (generation != generation_) {
        return;
    }
    transitionTo(nextState);
}

void ChannelSessionController::transitionTo(io::ChannelState nextState) {
    if (nextState == state_) {
        return;
    }

    // Mathematical invariant: passive disconnect occurs if and only if the channel
    // was in Open state and directly jumps to Closed or Error (without going through Closing).
    const bool isPassiveDisconnect = (state_ == io::ChannelState::Open)
        && (nextState == io::ChannelState::Closed || nextState == io::ChannelState::Error);

    state_ = nextState;

    if (state_ != io::ChannelState::Open) {
        if (autoSendTimer_ && autoSendTimer_->isActive()) {
            autoSendTimer_->stop();
            emit autoSendStateChanged(false);
        }
    }

    updateSnapshot(true);

    if (isPassiveDisconnect) {
        emit unexpectedDisconnectDetected();
    }
}

void ChannelSessionController::updateSnapshot(bool stateChangedFlag) {
    currentSnapshot_.state = state_;
    currentSnapshot_.stateText = formatChannelState(state_, activeRole_);
    currentSnapshot_.stateChanged = stateChangedFlag;
    currentSnapshot_.protocolSelectorEnabled = (state_ == io::ChannelState::Closed || state_ == io::ChannelState::Error);
    currentSnapshot_.serialControlLinesEnabled = (state_ == io::ChannelState::Open && activeRole_ == ChannelRole::Serial);

    if (state_ != io::ChannelState::Open) {
        currentSnapshot_.sendControlsEnabled = false;
        if (state_ == io::ChannelState::Opening) {
            currentSnapshot_.sendDisabledTooltip = tr("Connecting...");
        } else if (state_ == io::ChannelState::Closing) {
            currentSnapshot_.sendDisabledTooltip = tr("Disconnecting...");
        } else {
            currentSnapshot_.sendDisabledTooltip = tr("Connect first to send data");
        }
    } else {
        if (activeRole_ == ChannelRole::TcpServer) {
            currentSnapshot_.sendControlsEnabled = serverHasTargets_;
            if (!serverHasTargets_) {
                currentSnapshot_.sendDisabledTooltip = serverBroadcastMode_
                    ? tr("No connected clients")
                    : tr("Select at least one client");
            } else {
                currentSnapshot_.sendDisabledTooltip.clear();
            }
        } else {
            currentSnapshot_.sendControlsEnabled = true;
            currentSnapshot_.sendDisabledTooltip.clear();
        }
    }

    emit snapshotChanged(currentSnapshot_);
}

QString ChannelSessionController::formatChannelState(io::ChannelState state, ChannelRole role) {
    switch (state) {
    case io::ChannelState::Closed:
        return (role == ChannelRole::TcpServer) ? tr("Stopped") : tr("Disconnected");
    case io::ChannelState::Opening:
        return (role == ChannelRole::TcpServer) ? tr("Starting...") : tr("Connecting...");
    case io::ChannelState::Open:
        if (role == ChannelRole::TcpServer) {
            return tr("Listening");
        }
        if (role == ChannelRole::Udp) {
            return tr("Bound");
        }
        return tr("Connected");
    case io::ChannelState::Closing:
        return (role == ChannelRole::TcpServer) ? tr("Stopping...") : tr("Disconnecting...");
    case io::ChannelState::Error:
        return tr("Error");
    }
    return tr("Unknown");
}

QString ChannelSessionController::formatChannelError(io::ChannelErrorCode code, const QString& detail) {
    QString baseMsg;
    switch (code) {
    case io::ChannelErrorCode::None:
        return detail;
    case io::ChannelErrorCode::ConnectionFailed:
        baseMsg = tr("Connection failed");
        break;
    case io::ChannelErrorCode::Timeout:
        baseMsg = tr("Connection timeout");
        break;
    case io::ChannelErrorCode::WriteFailed:
        baseMsg = tr("Write failed");
        break;
    case io::ChannelErrorCode::ReadFailed:
        baseMsg = tr("Read failed");
        break;
    case io::ChannelErrorCode::PortNotFound:
        baseMsg = tr("Port not found");
        break;
    case io::ChannelErrorCode::PermissionDenied:
        baseMsg = tr("Permission denied");
        break;
    case io::ChannelErrorCode::ConnectionReset:
        baseMsg = tr("Connection reset");
        break;
    case io::ChannelErrorCode::Unknown:
    default:
        baseMsg = tr("Unknown error");
        break;
    }

    if (detail.isEmpty() || detail == baseMsg) {
        return baseMsg;
    }
    return QStringLiteral("%1 (%2)").arg(baseMsg, detail);
}

} // namespace ui::application
