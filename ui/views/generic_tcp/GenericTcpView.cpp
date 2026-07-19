/**
 * @file GenericTcpView.cpp
 * @brief Implementation of GenericTcpView.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "GenericTcpView.h"
#include "Config.h"
#include "../../../core/common/ISettingsService.h"
#include "../../widgets/TcpClientConnectionWidget.h"
#include "../../widgets/TcpServerConnectionWidget.h"
#include "../../widgets/UdpConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../widgets/ServerClientPanel.h"
#include "../../common/ConnectionAlert.h"
#include "../../common/TcpStateTransition.h"
#include "../../../infra/io/ChannelOperationWorker.h"
#include "../../../infra/io/ServerChannelWorker.h"
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QSplitter>
#include <QThread>
#include <QTimer>
#include <QMetaObject>
#include <QEvent>
#include <spdlog/spdlog.h>
#include "../../../infra/io/IChannel.h"

namespace ui::views::generic_tcp {

namespace {

constexpr auto kTcpClientText = QT_TRANSLATE_NOOP("ui::views::generic_tcp::Protocol", "TCP Client");
constexpr auto kTcpServerText = QT_TRANSLATE_NOOP("ui::views::generic_tcp::Protocol", "TCP Server");
constexpr auto kUdpText = QT_TRANSLATE_NOOP("ui::views::generic_tcp::Protocol", "UDP");

void populateProtocolOptions(QComboBox* combo) {
    if (!combo) return;
    const int currentValue = combo->count() > 0
        ? combo->currentData().toInt()
        : static_cast<int>(GenericTcpView::Protocol::TcpClient);
    combo->clear();
    combo->addItem(QCoreApplication::translate("ui::views::generic_tcp::Protocol", kTcpClientText),
                   static_cast<int>(GenericTcpView::Protocol::TcpClient));
    combo->addItem(QCoreApplication::translate("ui::views::generic_tcp::Protocol", kTcpServerText),
                   static_cast<int>(GenericTcpView::Protocol::TcpServer));
    combo->addItem(QCoreApplication::translate("ui::views::generic_tcp::Protocol", kUdpText),
                   static_cast<int>(GenericTcpView::Protocol::Udp));
    const int currentIndex = combo->findData(currentValue);
    combo->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
}

} // namespace

GenericTcpView::GenericTcpView(core::common::ISettingsService* settingsService, QWidget *parent)
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

    // 1. Protocol selector + Connection widgets in a stacked widget
    auto* connGroup = new QWidget(this);
    auto* connLayout = new QHBoxLayout(connGroup);
    connLayout->setContentsMargins(0, 0, 0, 0);
    connLayout->setSpacing(2);

    protocolCombo_ = new QComboBox(this);
    populateProtocolOptions(protocolCombo_);
    protocolCombo_->setFixedWidth(104);
    connLayout->addWidget(protocolCombo_);

    connectionStack_ = new QStackedWidget(this);
    connectionStack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    tcpClientWidget_ = new widgets::TcpClientConnectionWidget(settingsService_, connectionStack_);
    tcpClientWidget_->setSettingsGroup(QStringLiteral("tcp_client"));
    tcpClientWidget_->setDefaultPort(config::Network::kDefaultGenericTcpPort);
    connectionStack_->addWidget(tcpClientWidget_);

    tcpServerWidget_ = new widgets::TcpServerConnectionWidget(settingsService_, connectionStack_);
    tcpServerWidget_->setSettingsGroup(QStringLiteral("tcp_server"));
    tcpServerWidget_->setDefaultPort(config::Network::kDefaultGenericTcpPort);
    connectionStack_->addWidget(tcpServerWidget_);

    udpWidget_ = new widgets::UdpConnectionWidget(settingsService_, connectionStack_);
    udpWidget_->setSettingsGroup(QStringLiteral("udp"));
    udpWidget_->setDefaultPort(config::Network::kDefaultGenericTcpPort);
    connectionStack_->addWidget(udpWidget_);

    connectionStack_->setCurrentIndex(0);
    connLayout->addWidget(connectionStack_);
    mainLayout->addWidget(connGroup);

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

    // Protocol combo signal
    connect(protocolCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GenericTcpView::onProtocolChanged);

    // Client mode connections
    connect(tcpClientWidget_, &widgets::TcpClientConnectionWidget::connectClicked,
            this, &GenericTcpView::onConnectClicked);
    connect(tcpClientWidget_, &widgets::TcpClientConnectionWidget::disconnectClicked,
            this, &GenericTcpView::onDisconnectClicked);

    // Server mode connections
    connect(tcpServerWidget_, &widgets::TcpServerConnectionWidget::startListenClicked,
            this, &GenericTcpView::onStartListenClicked);
    connect(tcpServerWidget_, &widgets::TcpServerConnectionWidget::stopListenClicked,
            this, &GenericTcpView::onStopListenClicked);

    // UDP mode connections
    connect(udpWidget_, &widgets::UdpConnectionWidget::bindClicked,
            this, &GenericTcpView::onBindClicked);
    connect(udpWidget_, &widgets::UdpConnectionWidget::unbindClicked,
            this, &GenericTcpView::onUnbindClicked);

    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested,
            this, &GenericTcpView::onSendRequested);
    connect(serverClientPanel_, &widgets::ServerClientPanel::disconnectClientsRequested,
            this, &GenericTcpView::onDisconnectSelectedClientsRequested);
    connect(serverClientPanel_, &widgets::ServerClientPanel::disconnectAllClientsRequested,
            this, &GenericTcpView::onDisconnectAllClientsRequested);

    retranslateUi();

    reconnectTimer_ = new QTimer(this);
    reconnectTimer_->setSingleShot(true);
    connect(reconnectTimer_, &QTimer::timeout, this, &GenericTcpView::onReconnectTimerTick);

    onProtocolChanged(protocolCombo_->currentIndex());
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
    stopWorkerPair(thread, serverWorker);
}

void GenericTcpView::switchToProtocol(Protocol protocol) {
    currentProtocol_ = protocol;

    switch (protocol) {
    case Protocol::TcpClient:
        connectionStack_->setCurrentWidget(tcpClientWidget_);
        monitor_->setSettingsGroup(QStringLiteral("tcp_client/traffic"));
        inputSection_->setSettingsKey(QStringLiteral("tcp_client/ui/inputCollapsed"));
        inputWidget_->setSettingsGroup(QStringLiteral("tcp_client/input"));
        if (serverClientPanel_) {
            serverClientPanel_->clearClients();
            serverClientPanel_->hide();
        }
        break;
    case Protocol::TcpServer:
        connectionStack_->setCurrentWidget(tcpServerWidget_);
        monitor_->setSettingsGroup(QStringLiteral("tcp_server/traffic"));
        inputSection_->setSettingsKey(QStringLiteral("tcp_server/ui/inputCollapsed"));
        inputWidget_->setSettingsGroup(QStringLiteral("tcp_server/input"));
        if (serverClientPanel_) {
            serverClientPanel_->show();
        }
        break;
    case Protocol::Udp:
        connectionStack_->setCurrentWidget(udpWidget_);
        monitor_->setSettingsGroup(QStringLiteral("udp/traffic"));
        inputSection_->setSettingsKey(QStringLiteral("udp/ui/inputCollapsed"));
        inputWidget_->setSettingsGroup(QStringLiteral("udp/input"));
        if (serverClientPanel_) {
            serverClientPanel_->clearClients();
            serverClientPanel_->hide();
        }
        break;
    }
}

void GenericTcpView::onProtocolChanged(int index) {
    if (isConnected_) return;
    currentProtocol_ = static_cast<Protocol>(protocolCombo_->itemData(index).toInt());
    switchToProtocol(currentProtocol_);
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
    tcpClientWidget_->setDisplayState(widgets::TcpClientConnectionWidget::DisplayState::Connecting);

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
    tcpServerWidget_->setDisplayState(widgets::TcpServerConnectionWidget::DisplayState::Connecting);

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
    tcpServerWidget_->setDisplayState(widgets::TcpServerConnectionWidget::DisplayState::Disconnecting);
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
    udpWidget_->setDisplayState(widgets::UdpConnectionWidget::DisplayState::Connecting);

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
    udpWidget_->setDisplayState(widgets::UdpConnectionWidget::DisplayState::Disconnecting);
    QMetaObject::invokeMethod(worker_, "close", Qt::QueuedConnection);
}

void GenericTcpView::onSendRequested(const QByteArray& data) {
    if (!isConnected_) {
        ui::common::connection_alert::showNotConnected(this);
        return;
    }

    switch (currentProtocol_) {
    case Protocol::TcpClient:
    case Protocol::Udp:
        GenericChannelViewBase::onSendRequested(data);
        break;
    case Protocol::TcpServer:
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

void GenericTcpView::onWorkerStateChanged(io::ChannelState state, quint64 generation) {
    if (generation != connectionGeneration_) {
        return;
    }

    const bool wasConnected = isConnected_;
    const auto transition = ui::common::computeTcpStateTransition(
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

    using DisplayState = widgets::BaseConnectionWidget::DisplayState;

    switch (state) {
    case io::ChannelState::Opening:
        if (currentProtocol_ == Protocol::TcpClient) {
            tcpClientWidget_->setDisplayState(DisplayState::Connecting);
        } else {
            udpWidget_->setDisplayState(DisplayState::Connecting);
        }
        break;
    case io::ChannelState::Open:
        if (currentProtocol_ == Protocol::TcpClient) {
            tcpClientWidget_->setConnected(true);
        } else {
            udpWidget_->setConnected(true);
        }
        break;
    case io::ChannelState::Closing:
        if (currentProtocol_ == Protocol::TcpClient) {
            tcpClientWidget_->setDisplayState(DisplayState::Disconnecting);
        } else {
            udpWidget_->setDisplayState(DisplayState::Disconnecting);
        }
        break;
    case io::ChannelState::Closed:
    case io::ChannelState::Error:
        if (currentProtocol_ == Protocol::TcpClient) {
            tcpClientWidget_->setConnected(false);
        } else {
            udpWidget_->setConnected(false);
        }
        break;
    }

    if (monitor_) {
        monitor_->appendInfo(tr("State changed: %1").arg(stateStr));
    }
    if (transition.showDisconnectAlert) {
        ui::common::connection_alert::showDisconnected(this);
    }

    auto* activeWidget = (currentProtocol_ == Protocol::TcpClient)
        ? static_cast<widgets::NetworkConnectionWidget*>(tcpClientWidget_)
        : static_cast<widgets::NetworkConnectionWidget*>(udpWidget_);

    if (!isConnected_ && wasConnected
        && currentProtocol_ == Protocol::TcpClient
        && activeWidget->autoReconnectEnabled()
        && !suppressDisconnectAlert_
        && !reconnectTimer_->isActive()) {
        startReconnectTimer(activeWidget);
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

    using DisplayState = widgets::BaseConnectionWidget::DisplayState;

    switch (state) {
    case io::ChannelState::Opening:
        tcpServerWidget_->setDisplayState(DisplayState::Connecting);
        break;
    case io::ChannelState::Open:
        tcpServerWidget_->setConnected(true);
        break;
    case io::ChannelState::Closing:
        tcpServerWidget_->setDisplayState(DisplayState::Disconnecting);
        break;
    case io::ChannelState::Closed:
    case io::ChannelState::Error:
        if (serverClientPanel_) {
            serverClientPanel_->clearClients();
        }
        tcpServerWidget_->setConnected(false);
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
    populateProtocolOptions(protocolCombo_);
}

void GenericTcpView::onReconnectTimerTick() {
    auto* activeWidget = tcpClientWidget_;
    if (!activeWidget || !worker_) return;

    if (!activeWidget->autoReconnectEnabled()) {
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

    suppressDisconnectAlert_ = false;
    const quint64 generation = ++connectionGeneration_;
    if (monitor_) {
        monitor_->appendInfo(tr("Auto-reconnecting to %1:%2 (attempt %3)...")
                                 .arg(reconnectHost_)
                                 .arg(reconnectPort_)
                                 .arg(reconnectPolicy_.attemptCount()));
    }
    activeWidget->setDisplayState(widgets::BaseConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker_, "openTcp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, reconnectHost_),
                              Q_ARG(int, reconnectPort_),
                              Q_ARG(quint64, generation));

    reconnectTimer_->setInterval(reconnectPolicy_.nextDelayMs());
    reconnectTimer_->start();
}

} // namespace ui::views::generic_tcp
