/**
 * @file GenericTcpView.cpp
 * @brief Implementation of GenericTcpView.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "GenericTcpView.h"
#include "AppConstants.h"
#include "../../common/ISettingsService.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../widgets/ServerClientPanel.h"
#include "../../common/ConnectionAlert.h"
#include "../../common/TcpConnectionStateCoordinator.h"
#include "../../../infra/io/ChannelOperationWorker.h"
#include "../../../infra/io/ServerChannelWorker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QThread>
#include <QTimer>
#include <QMetaObject>
#include <QEvent>
#include <spdlog/spdlog.h>
#include "../../../infra/io/IChannel.h"

namespace ui::views::generic_tcp {

GenericTcpView::GenericTcpView(ui::common::ISettingsService* settingsService, QWidget *parent)
    : GenericChannelViewBase(settingsService, parent) {
    setupUi();
    startWorker();
    startServerWorker();
}

GenericTcpView::~GenericTcpView() noexcept {
    stopServerWorker();
    stopWorker();
}

void GenericTcpView::startWorker() {
    GenericChannelViewBase::startWorker();
    if (worker_) {
        connect(worker_, &io::ChannelOperationWorker::stateChangedWithGeneration, 
                this, &GenericTcpView::onWorkerStateChanged);
    }
}

void GenericTcpView::setupUi() {
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    // 1. Connection Section (Top)
    connectionWidget_ = new widgets::TcpConnectionWidget(settingsService_, this);
    connectionWidget_->setSettingsGroup(QStringLiteral("tcp_client"));
    connectionWidget_->setDefaultPort(app::constants::Values::Network::kDefaultGenericTcpPort);
    mainLayout->addWidget(connectionWidget_);

    // 2. Central Area (Traffic Monitor + Server Client Panel)
    auto* centerSplitter = new QSplitter(Qt::Horizontal, this);
    centerSplitter->setChildrenCollapsible(false);

    monitor_ = new widgets::ByteMonitorWidget(settingsService_, centerSplitter);
    monitor_->setSettingsGroup(QStringLiteral("tcp_client/traffic"));
    centerSplitter->addWidget(monitor_);

    serverClientPanel_ = new widgets::ServerClientPanel(centerSplitter);
    serverClientPanel_->setObjectName(QStringLiteral("serverClientPanel"));
    serverClientPanel_->hide();
    centerSplitter->addWidget(serverClientPanel_);
    centerSplitter->setStretchFactor(0, 1);
    centerSplitter->setStretchFactor(1, 0);
    mainLayout->addWidget(centerSplitter);

    // 3. Input Section (Bottom)
    inputSection_ = new widgets::CollapsibleSection(settingsService_, this);
    inputSection_->setSettingsKey(QStringLiteral("tcp_client/ui/inputCollapsed"));
    auto inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputWidget_ = new widgets::GenericInputWidget(settingsService_, inputSection_->contentWidget());
    inputWidget_->setSettingsGroup(QStringLiteral("tcp_client/input"));
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputSection_);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
    mainLayout->setStretch(2, 0);

    // Client mode connections
    connect(connectionWidget_, &widgets::TcpConnectionWidget::connectClicked,
            this, &GenericTcpView::onConnectClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
            this, &GenericTcpView::onDisconnectClicked);

    // Server mode connections
    connect(connectionWidget_, &widgets::TcpConnectionWidget::startListenClicked,
            this, &GenericTcpView::onStartListenClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::stopListenClicked,
            this, &GenericTcpView::onStopListenClicked);

    // UDP mode connections
    connect(connectionWidget_, &widgets::TcpConnectionWidget::bindClicked,
            this, &GenericTcpView::onBindClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::unbindClicked,
            this, &GenericTcpView::onUnbindClicked);
    connect(connectionWidget_, &widgets::TcpConnectionWidget::protocolChanged,
            this, &GenericTcpView::onProtocolChanged);

    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested,
            this, &GenericTcpView::onSendRequested);
    connect(inputWidget_, &widgets::GenericInputWidget::fileSendRequested,
            this, &GenericTcpView::onFileSendRequested);
    connect(serverClientPanel_, &widgets::ServerClientPanel::disconnectClientsRequested,
            this, &GenericTcpView::onDisconnectSelectedClientsRequested);
    connect(serverClientPanel_, &widgets::ServerClientPanel::disconnectAllClientsRequested,
            this, &GenericTcpView::onDisconnectAllClientsRequested);

    retranslateUi();

    reconnectTimer_ = new QTimer(this);
    reconnectTimer_->setSingleShot(true);
    connect(reconnectTimer_, &QTimer::timeout, this, &GenericTcpView::onReconnectTimerTick);

    onProtocolChanged(connectionWidget_->currentProtocol());
}

void GenericTcpView::startServerWorker() {
    serverThread_ = new QThread();
    auto* serverWorker = new io::ServerChannelWorker();
    serverWorker->moveToThread(serverThread_);
    connect(serverThread_, &QThread::finished, serverThread_, &QObject::deleteLater);

    connect(serverWorker, &io::ServerChannelWorker::clientConnected,
            this, &GenericTcpView::onServerClientConnected);
    connect(serverWorker, &io::ServerChannelWorker::clientDisconnected,
            this, &GenericTcpView::onServerClientDisconnected);
    connect(serverWorker, &io::ServerChannelWorker::monitorWithClient,
            this, &GenericTcpView::onServerMonitorWithClient);
    connect(serverWorker, &io::ServerChannelWorker::stateChanged,
            this, &GenericTcpView::onServerStateChanged);
    connect(serverWorker, &io::ServerChannelWorker::channelErrorOccurred,
            this, &GenericTcpView::onServerError);

    serverThread_->start();
    serverWorker_ = serverWorker;
}

void GenericTcpView::stopServerWorker() {
    auto* thread = serverThread_;
    auto* serverWorker = serverWorker_;
    serverThread_ = nullptr;
    serverWorker_ = nullptr;

    if (!thread) return;

    if (!serverWorker) {
        if (thread->isRunning()) {
            thread->quit();
        } else {
            delete thread;
        }
        return;
    }

    serverWorker->disconnect(this);

    if (!thread->isRunning()) {
        delete serverWorker;
        delete thread;
        return;
    }

    QMetaObject::invokeMethod(serverWorker, [serverWorker, thread]() {
        QObject::connect(serverWorker, &QObject::destroyed, thread, &QThread::quit, Qt::UniqueConnection);
        serverWorker->deleteLater();
    }, Qt::QueuedConnection);
}

void GenericTcpView::switchToClientMode() {
    currentProtocol_ = widgets::TcpConnectionWidget::Protocol::TcpClient;
    connectionWidget_->setSettingsGroup(QStringLiteral("tcp_client"));
    connectionWidget_->setDefaultPort(app::constants::Values::Network::kDefaultGenericTcpPort);
    monitor_->setSettingsGroup(QStringLiteral("tcp_client/traffic"));
    inputSection_->setSettingsKey(QStringLiteral("tcp_client/ui/inputCollapsed"));
    inputWidget_->setSettingsGroup(QStringLiteral("tcp_client/input"));
    if (serverClientPanel_) {
        serverClientPanel_->clearClients();
        serverClientPanel_->hide();
    }
}

void GenericTcpView::switchToServerMode() {
    currentProtocol_ = widgets::TcpConnectionWidget::Protocol::TcpServer;
    connectionWidget_->setSettingsGroup(QStringLiteral("tcp_server"));
    connectionWidget_->setDefaultPort(app::constants::Values::Network::kDefaultGenericTcpPort);
    monitor_->setSettingsGroup(QStringLiteral("tcp_server/traffic"));
    inputSection_->setSettingsKey(QStringLiteral("tcp_server/ui/inputCollapsed"));
    inputWidget_->setSettingsGroup(QStringLiteral("tcp_server/input"));
    if (serverClientPanel_) {
        serverClientPanel_->show();
    }
}

void GenericTcpView::switchToUdpMode() {
    currentProtocol_ = widgets::TcpConnectionWidget::Protocol::Udp;
    connectionWidget_->setSettingsGroup(QStringLiteral("udp"));
    connectionWidget_->setDefaultPort(app::constants::Values::Network::kDefaultGenericTcpPort);
    monitor_->setSettingsGroup(QStringLiteral("udp/traffic"));
    inputSection_->setSettingsKey(QStringLiteral("udp/ui/inputCollapsed"));
    inputWidget_->setSettingsGroup(QStringLiteral("udp/input"));
    if (serverClientPanel_) {
        serverClientPanel_->clearClients();
        serverClientPanel_->hide();
    }
}

void GenericTcpView::onConnectClicked(const QString& ip, int port) {
    if (!worker_) return;

    stopReconnectTimer();
    reconnectPolicy_.reset();
    reconnectHost_ = ip;
    reconnectPort_ = port;

    spdlog::info("GenericTcp: Connecting to {}:{}", ip.toStdString(), port);
    suppressDisconnectAlert_ = false;
    const quint64 generation = ++connectionGeneration_;
    if (monitor_) {
        monitor_->appendInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    }
    connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker_, "openTcp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, ip),
                              Q_ARG(int, port),
                              Q_ARG(quint64, generation));
}

void GenericTcpView::onStartListenClicked(const QString& ip, int port) {
    if (!serverWorker_) return;

    spdlog::info("GenericTcp: Starting TCP server on {}:{}", ip.toStdString(), port);
    if (monitor_) {
        monitor_->appendInfo(tr("Starting TCP server on %1:%2...").arg(ip).arg(port));
    }
    connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(serverWorker_, "openTcpServer",
                              Qt::QueuedConnection,
                              Q_ARG(QString, ip),
                              Q_ARG(int, port),
                              Q_ARG(int, 0));
}

void GenericTcpView::onStopListenClicked() {
    if (!serverWorker_) return;

    if (monitor_) {
        monitor_->appendInfo(tr("Stopping TCP server..."));
    }
    connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Disconnecting);
    QMetaObject::invokeMethod(serverWorker_, "closeAllClients", Qt::QueuedConnection);
}

void GenericTcpView::onBindClicked(const QString& localIp, int localPort,
                                    const QString& remoteIp, int remotePort) {
    if (!worker_) return;

    spdlog::info("GenericTcp: Binding UDP {}:{}", localIp.toStdString(), localPort);
    if (monitor_) {
        if (!remoteIp.isEmpty()) {
            monitor_->appendInfo(tr("Binding UDP %1:%2 -> %3:%4...")
                                     .arg(localIp).arg(localPort)
                                     .arg(remoteIp).arg(remotePort));
        } else {
            monitor_->appendInfo(tr("Binding UDP %1:%2...").arg(localIp).arg(localPort));
        }
    }

    suppressDisconnectAlert_ = false;
    connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker_, "openUdp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, localIp),
                              Q_ARG(int, localPort),
                              Q_ARG(QString, remoteIp),
                              Q_ARG(int, remotePort));
}

void GenericTcpView::onUnbindClicked() {
    if (!worker_) return;

    suppressDisconnectAlert_ = true;
    connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Disconnecting);
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void GenericTcpView::onProtocolChanged(widgets::TcpConnectionWidget::Protocol protocol) {
    if (isConnected_) return;

    switch (protocol) {
    case widgets::TcpConnectionWidget::Protocol::TcpClient:
        switchToClientMode();
        break;
    case widgets::TcpConnectionWidget::Protocol::TcpServer:
        switchToServerMode();
        break;
    case widgets::TcpConnectionWidget::Protocol::Udp:
        switchToUdpMode();
        break;
    }
}

void GenericTcpView::onSendRequested(const QByteArray& data) {
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }

    switch (currentProtocol_) {
    case widgets::TcpConnectionWidget::Protocol::TcpClient:
    case widgets::TcpConnectionWidget::Protocol::Udp:
        GenericChannelViewBase::onSendRequested(data);
        break;
    case widgets::TcpConnectionWidget::Protocol::TcpServer:
        if (!serverWorker_ || !serverClientPanel_) {
            return;
        }
        {
            const QList<int> targetClientIds = serverClientPanel_->broadcastEnabled()
                ? serverClientPanel_->allClientIds()
                : serverClientPanel_->selectedClientIds();
            if (targetClientIds.isEmpty()) {
                if (monitor_) {
                    monitor_->appendWarn(serverClientPanel_->broadcastEnabled()
                                             ? tr("Server mode: no connected clients available")
                                             : tr("Server mode: select at least one client"));
                }
                return;
            }

            for (const int clientId : targetClientIds) {
                QMetaObject::invokeMethod(serverWorker_, "writeToClient",
                                          Qt::QueuedConnection,
                                          Q_ARG(int, clientId),
                                          Q_ARG(QByteArray, data));
            }
        }
        break;
    }
}

void GenericTcpView::onFileSendRequested(const QString& filePath) {
    if (!isConnected_) {
        ui::common::ConnectionAlert::showNotConnected(this);
        return;
    }

    switch (currentProtocol_) {
    case widgets::TcpConnectionWidget::Protocol::TcpClient:
    case widgets::TcpConnectionWidget::Protocol::Udp:
        GenericChannelViewBase::onFileSendRequested(filePath);
        break;
    case widgets::TcpConnectionWidget::Protocol::TcpServer:
        if (monitor_) {
            monitor_->appendWarn(tr("Server mode: file transfer not supported"));
        }
        break;
    }
}

void GenericTcpView::onWorkerStateChanged(io::ChannelState state, quint64 generation) {
    if (generation != connectionGeneration_) {
        return;
    }

    const bool wasConnected = isConnected_;
    const auto transition = ui::common::TcpConnectionStateCoordinator::transition(
        state,
        wasConnected,
        suppressDisconnectAlert_);

    isConnected_ = (state == io::ChannelState::Open);
    if (transition.clearSuppressDisconnectAlert) {
        suppressDisconnectAlert_ = false;
    }

    QString stateStr;
    switch (state) {
        case io::ChannelState::Closed: stateStr = tr("Closed"); break;
        case io::ChannelState::Opening: stateStr = tr("Opening"); break;
        case io::ChannelState::Open: stateStr = tr("Connected"); break;
        case io::ChannelState::Closing: stateStr = tr("Closing"); break;
        case io::ChannelState::Error: stateStr = tr("Error"); break;
        default: stateStr = tr("Unknown"); break;
    }

    switch (state) {
    case io::ChannelState::Opening:
        connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Connecting);
        break;
    case io::ChannelState::Open:
        connectionWidget_->setConnected(true);
        break;
    case io::ChannelState::Closing:
        connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Disconnecting);
        break;
    case io::ChannelState::Closed:
    case io::ChannelState::Error:
        connectionWidget_->setConnected(false);
        break;
    }

    if (monitor_) {
        monitor_->appendInfo(tr("State changed: %1").arg(stateStr));
    }
    if (transition.showDisconnectAlert) {
        ui::common::ConnectionAlert::showDisconnected(this);
    }

    if (!isConnected_ && wasConnected
        && currentProtocol_ == widgets::TcpConnectionWidget::Protocol::TcpClient
        && connectionWidget_->autoReconnectEnabled()
        && !suppressDisconnectAlert_
        && !reconnectTimer_->isActive()) {
        startReconnectTimer(connectionWidget_);
    }

    if (state == io::ChannelState::Open) {
        reconnectPolicy_.onSuccess();
    }
}

void GenericTcpView::onServerClientConnected(int clientId, const QString& peerInfo) {
    if (serverClientPanel_) {
        serverClientPanel_->addOrUpdateClient(clientId, peerInfo);
    }
    if (monitor_) {
        monitor_->appendInfo(tr("Client #%1 connected: %2").arg(clientId).arg(peerInfo));
    }
}

void GenericTcpView::onServerClientDisconnected(int clientId) {
    const bool removed = serverClientPanel_ ? serverClientPanel_->removeClient(clientId) : true;
    if (removed && monitor_) {
        monitor_->appendInfo(tr("Client #%1 disconnected").arg(clientId));
    }
}

void GenericTcpView::onServerMonitorWithClient(bool isTx, const QByteArray& data, int clientId) {
    if (monitor_) {
        monitor_->appendMessageWithClient(isTx, data, clientId);
    }
}

void GenericTcpView::onServerStateChanged(io::ChannelState state) {
    isConnected_ = (state == io::ChannelState::Open);

    QString stateStr;
    switch (state) {
        case io::ChannelState::Open: stateStr = tr("Listening"); break;
        case io::ChannelState::Closed: stateStr = tr("Stopped"); break;
        case io::ChannelState::Error: stateStr = tr("Error"); break;
        default: stateStr = tr("Unknown"); break;
    }

    switch (state) {
    case io::ChannelState::Opening:
        connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Connecting);
        break;
    case io::ChannelState::Open:
        connectionWidget_->setConnected(true);
        break;
    case io::ChannelState::Closing:
        connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Disconnecting);
        break;
    case io::ChannelState::Closed:
    case io::ChannelState::Error:
        if (serverClientPanel_) {
            serverClientPanel_->clearClients();
        }
        connectionWidget_->setConnected(false);
        break;
    }

    if (monitor_) {
        monitor_->appendInfo(tr("Server state: %1").arg(stateStr));
    }
}

void GenericTcpView::onServerError(const QString& deviceHint, const QString& error) {
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("TCP Server") : deviceHint;
    if (monitor_) {
        monitor_->appendError(tr("Server Error: %1").arg(error));
    }
    spdlog::error("{} Error: {}", hint.toStdString(), error.toStdString());
}

void GenericTcpView::onDisconnectSelectedClientsRequested(const QList<int>& clientIds)
{
    if (!serverWorker_) {
        return;
    }

    for (const int clientId : clientIds) {
        QMetaObject::invokeMethod(serverWorker_, "closeClient",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, clientId));
    }
}

void GenericTcpView::onDisconnectAllClientsRequested()
{
    if (!serverWorker_ || !serverClientPanel_) {
        return;
    }

    const QList<int> clientIds = serverClientPanel_->allClientIds();
    for (const int clientId : clientIds) {
        QMetaObject::invokeMethod(serverWorker_, "closeClient",
                                  Qt::QueuedConnection,
                                  Q_ARG(int, clientId));
    }
}

void GenericTcpView::retranslateUi() {
    if (inputSection_) inputSection_->setTitle(tr("Send Data"));
}

void GenericTcpView::onReconnectTimerTick() {
    if (!connectionWidget_ || !worker_) return;

    if (!connectionWidget_->autoReconnectEnabled()) {
        stopReconnectTimer();
        return;
    }

    if (reconnectHost_.isEmpty()) {
        stopReconnectTimer();
        return;
    }

    spdlog::info("GenericTcp: Auto-reconnecting to {}:{} (attempt {})",
                 reconnectHost_.toStdString(), reconnectPort_,
                 reconnectPolicy_.attemptCount());

    if (monitor_) {
        monitor_->appendInfo(tr("Reconnecting to %1:%2...").arg(reconnectHost_).arg(reconnectPort_));
    }
    suppressDisconnectAlert_ = false;
    const quint64 generation = ++connectionGeneration_;
    connectionWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker_, "openTcp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, reconnectHost_),
                              Q_ARG(int, reconnectPort_),
                              Q_ARG(quint64, generation));
}

} // namespace ui::views::generic_tcp
