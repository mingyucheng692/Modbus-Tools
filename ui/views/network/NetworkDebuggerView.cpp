/**
 * @file NetworkDebuggerView.cpp
 * @brief Implementation of NetworkDebuggerView.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "NetworkDebuggerView.h"
#include "Config.h"
#include "infra/config/ISettingsService.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/UdpConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../widgets/ServerClientPanel.h"
#include "../../common/ConnectionAlert.h"
#include "../../../infra/io/ChannelOperationWorker.h"
#include "../../../infra/io/ServerChannelWorker.h"
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QSplitter>
#include <QMetaObject>
#include <QThread>
#include <QEvent>
#include <spdlog/spdlog.h>
#include "../../../infra/io/IChannel.h"

namespace ui::views::network {

namespace {

/// In-memory state transition flags for TCP connection state changes.
/// Previously a separate header (TcpStateTransition.h); inlined here as the
/// sole consumer.
struct TcpConnectionStateTransition {
    bool setConnected = false;
    bool setDisconnected = false;
    bool clearSuppressDisconnectAlert = false;
    bool showDisconnectAlert = false;
};

TcpConnectionStateTransition computeTcpStateTransition(io::ChannelState state,
                                                       bool wasConnected,
                                                       bool suppressDisconnectAlert) {
    TcpConnectionStateTransition result;
    if (state == io::ChannelState::Open) {
        result.setConnected = !wasConnected;
        result.clearSuppressDisconnectAlert = true;
        return result;
    }
    if (state == io::ChannelState::Closed && wasConnected) {
        result.setDisconnected = true;
        result.showDisconnectAlert = !suppressDisconnectAlert;
    }
    return result;
}

constexpr auto kTcpClientText = QT_TRANSLATE_NOOP("ui::views::network::Protocol", "TCP Client");
constexpr auto kTcpServerText = QT_TRANSLATE_NOOP("ui::views::network::Protocol", "TCP Server");
constexpr auto kUdpText = QT_TRANSLATE_NOOP("ui::views::network::Protocol", "UDP");

void populateProtocolOptions(QComboBox* combo) {
    if (!combo) return;
    const int currentValue = combo->count() > 0
        ? combo->currentData().toInt()
        : static_cast<int>(NetworkDebuggerView::Protocol::TcpClient);
    combo->clear();
    combo->addItem(QCoreApplication::translate("ui::views::network::Protocol", kTcpClientText),
                   static_cast<int>(NetworkDebuggerView::Protocol::TcpClient));
    combo->addItem(QCoreApplication::translate("ui::views::network::Protocol", kTcpServerText),
                   static_cast<int>(NetworkDebuggerView::Protocol::TcpServer));
    combo->addItem(QCoreApplication::translate("ui::views::network::Protocol", kUdpText),
                   static_cast<int>(NetworkDebuggerView::Protocol::Udp));
    const int currentIndex = combo->findData(currentValue);
    combo->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
}

} // namespace

NetworkDebuggerView::NetworkDebuggerView(infra::config::ISettingsService* settingsService, QWidget *parent)
    : GenericChannelViewBase(settingsService, parent),
      channelCtrl_(this) {
    channelController_ = &channelCtrl_;
    setupUi();
    startWorker();
    startServerWorker();
}

NetworkDebuggerView::~NetworkDebuggerView() noexcept {
    stopServerWorker();
    // channelCtrl_ is a value member, destroyed automatically.
}

void NetworkDebuggerView::startWorker() {
    auto* worker = channelCtrl_.createWorker();
    connect(worker, &io::ChannelOperationWorker::channelErrorOccurred,
            this, &NetworkDebuggerView::onWorkerError);
    connect(worker, &io::ChannelOperationWorker::monitor,
            this, &NetworkDebuggerView::onWorkerMonitor);
    connect(worker, &io::ChannelOperationWorker::stateChangedWithGeneration,
            this, &NetworkDebuggerView::onWorkerStateChanged);
}

void NetworkDebuggerView::setupUi() {
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

    tcpClientWidget_ = new widgets::TcpConnectionWidget(widgets::TcpRole::Client, settingsService_, connectionStack_);
    tcpClientWidget_->setSettingsGroup(QStringLiteral("network_debugger/client"));
    tcpClientWidget_->setDefaultPort(config::Network::kDefaultNetworkDebuggerPort);
    connectionStack_->addWidget(tcpClientWidget_);

    tcpServerWidget_ = new widgets::TcpConnectionWidget(widgets::TcpRole::Server, settingsService_, connectionStack_);
    tcpServerWidget_->setSettingsGroup(QStringLiteral("network_debugger/server"));
    tcpServerWidget_->setDefaultPort(config::Network::kDefaultNetworkDebuggerPort);
    connectionStack_->addWidget(tcpServerWidget_);

    udpWidget_ = new widgets::UdpConnectionWidget(settingsService_, connectionStack_);
    udpWidget_->setSettingsGroup(QStringLiteral("network_debugger/udp"));
    udpWidget_->setDefaultPort(config::Network::kDefaultNetworkDebuggerPort);
    connectionStack_->addWidget(udpWidget_);

    connectionStack_->setCurrentIndex(0);
    connLayout->addWidget(connectionStack_);
    mainLayout->addWidget(connGroup);

    // 2. Central Area (Traffic Monitor + Server Client Panel)
    auto* centerSplitter = new QSplitter(Qt::Horizontal, this);
    centerSplitter->setChildrenCollapsible(false);

    monitor_ = new widgets::ByteMonitorWidget(settingsService_, centerSplitter);
    monitor_->setSettingsGroup(QStringLiteral("network_debugger/client/traffic"));
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
    inputSection_->setSettingsKey(QStringLiteral("network_debugger/client/ui/inputCollapsed"));
    auto inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputWidget_ = new widgets::GenericInputWidget(settingsService_, inputSection_->contentWidget());
    inputWidget_->setSettingsGroup(QStringLiteral("network_debugger/client/input"));
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputSection_);
    mainLayout->setStretch(0, 0);
    mainLayout->setStretch(1, 1);
    mainLayout->setStretch(2, 0);

    // Protocol combo signal
    connect(protocolCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NetworkDebuggerView::onProtocolChanged);

    // Client mode connections
    connect(tcpClientWidget_, &widgets::TcpConnectionWidget::connectClicked,
            this, &NetworkDebuggerView::onConnectClicked);
    connect(tcpClientWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
            this, &NetworkDebuggerView::onDisconnectClicked);

    // Server mode connections
    connect(tcpServerWidget_, &widgets::TcpConnectionWidget::startListenClicked,
            this, &NetworkDebuggerView::onStartListenClicked);
    connect(tcpServerWidget_, &widgets::TcpConnectionWidget::stopListenClicked,
            this, &NetworkDebuggerView::onStopListenClicked);

    // UDP mode connections
    connect(udpWidget_, &widgets::UdpConnectionWidget::bindClicked,
            this, &NetworkDebuggerView::onBindClicked);
    connect(udpWidget_, &widgets::UdpConnectionWidget::unbindClicked,
            this, &NetworkDebuggerView::onUnbindClicked);

    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested,
            this, &NetworkDebuggerView::onSendRequested);
    connect(serverClientPanel_, &widgets::ServerClientPanel::disconnectClientsRequested,
            this, &NetworkDebuggerView::onDisconnectSelectedClientsRequested);
    connect(serverClientPanel_, &widgets::ServerClientPanel::disconnectAllClientsRequested,
            this, &NetworkDebuggerView::onDisconnectAllClientsRequested);

    retranslateUi();

    // Reconnect timer is managed by ChannelController; connect its signal to our slot.
    connect(&channelCtrl_, &ChannelController::reconnectTimeout,
            this, &NetworkDebuggerView::onReconnectTimerTick);

    onProtocolChanged(protocolCombo_->currentIndex());
}

void NetworkDebuggerView::startServerWorker() {
    serverThread_ = new QThread();
    auto* serverWorker = new io::ServerChannelWorker();
    serverWorker->moveToThread(serverThread_);
    connect(serverThread_, &QThread::finished, serverThread_, &QObject::deleteLater);

    connect(serverWorker, &io::ServerChannelWorker::clientConnected,
            this, &NetworkDebuggerView::onServerClientConnected);
    connect(serverWorker, &io::ServerChannelWorker::clientDisconnected,
            this, &NetworkDebuggerView::onServerClientDisconnected);
    connect(serverWorker, &io::ServerChannelWorker::monitorWithClient,
            this, &NetworkDebuggerView::onServerMonitorWithClient);
    connect(serverWorker, &io::ServerChannelWorker::stateChanged,
            this, &NetworkDebuggerView::onServerStateChanged);
    connect(serverWorker, &io::ServerChannelWorker::channelErrorOccurred,
            this, &NetworkDebuggerView::onServerError);

    serverThread_->start();
    serverWorker_ = serverWorker;
}

void NetworkDebuggerView::stopServerWorker() {
    auto* thread = serverThread_;
    auto* serverWorker = serverWorker_;
    serverThread_ = nullptr;
    serverWorker_ = nullptr;
    // Explicit teardown on the server thread BEFORE deleteLater(): stopping
    // the listener and closing every client channel is a visible state
    // change (clientDisconnected emissions) that must not depend on the
    // destructor running during thread drain (NEW-B shutdown race).
    channelCtrl_.stopWorkerPair(thread, serverWorker, {}, [serverWorker](QObject*) {
        QMetaObject::invokeMethod(serverWorker, "closeAllClients",
                                  Qt::DirectConnection);
    });
}

void NetworkDebuggerView::switchToProtocol(Protocol protocol) {
    currentProtocol_ = protocol;

    switch (protocol) {
    case Protocol::TcpClient:
        connectionStack_->setCurrentWidget(tcpClientWidget_);
        monitor_->setSettingsGroup(QStringLiteral("network_debugger/client/traffic"));
        inputSection_->setSettingsKey(QStringLiteral("network_debugger/client/ui/inputCollapsed"));
        inputWidget_->setSettingsGroup(QStringLiteral("network_debugger/client/input"));
        if (serverClientPanel_) {
            serverClientPanel_->clearClients();
            serverClientPanel_->hide();
        }
        break;
    case Protocol::TcpServer:
        connectionStack_->setCurrentWidget(tcpServerWidget_);
        monitor_->setSettingsGroup(QStringLiteral("network_debugger/server/traffic"));
        inputSection_->setSettingsKey(QStringLiteral("network_debugger/server/ui/inputCollapsed"));
        inputWidget_->setSettingsGroup(QStringLiteral("network_debugger/server/input"));
        if (serverClientPanel_) {
            serverClientPanel_->show();
        }
        break;
    case Protocol::Udp:
        connectionStack_->setCurrentWidget(udpWidget_);
        monitor_->setSettingsGroup(QStringLiteral("network_debugger/udp/traffic"));
        inputSection_->setSettingsKey(QStringLiteral("network_debugger/udp/ui/inputCollapsed"));
        inputWidget_->setSettingsGroup(QStringLiteral("network_debugger/udp/input"));
        if (serverClientPanel_) {
            serverClientPanel_->clearClients();
            serverClientPanel_->hide();
        }
        break;
    }
}

void NetworkDebuggerView::onProtocolChanged(int index) {
    if (isConnected_) return;
    currentProtocol_ = static_cast<Protocol>(protocolCombo_->itemData(index).toInt());
    switchToProtocol(currentProtocol_);
}

void NetworkDebuggerView::onConnectClicked(const QString& ip, int port) {
    auto* worker = channelCtrl_.worker();
    if (!worker) return;

    channelCtrl_.stopReconnectTimer();
    channelCtrl_.resetReconnect();
    reconnectHost_ = ip;
    reconnectPort_ = port;

    SPDLOG_INFO("NetworkDebugger: Connecting to {}:{}", ip.toStdString(), port);
    suppressDisconnectAlert_ = false;
    // Fresh user intent: any prior manual disconnect is superseded, so a
    // later passive loss is allowed to trigger the reconnect loop again.
    manualDisconnectRequested_ = false;
    const quint64 generation = ++connectionGeneration_;
    if (monitor_) {
        monitor_->appendInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    }
    tcpClientWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker, "openTcp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, ip),
                              Q_ARG(int, port),
                              Q_ARG(quint64, generation));
}

void NetworkDebuggerView::onStartListenClicked(const QString& ip, int port) {
    if (!serverWorker_) return;

    SPDLOG_INFO("NetworkDebugger: Starting TCP server on {}:{}", ip.toStdString(), port);
    if (monitor_) {
        monitor_->appendInfo(tr("Starting TCP server on %1:%2...").arg(ip).arg(port));
    }
    tcpServerWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(serverWorker_, "openTcpServer",
                              Qt::QueuedConnection,
                              Q_ARG(QString, ip),
                              Q_ARG(int, port),
                              Q_ARG(int, 0));
}

void NetworkDebuggerView::onStopListenClicked() {
    if (!serverWorker_) return;

    if (monitor_) {
        monitor_->appendInfo(tr("Stopping TCP server..."));
    }
    tcpServerWidget_->setDisplayState(widgets::TcpConnectionWidget::DisplayState::Disconnecting);
    QMetaObject::invokeMethod(serverWorker_, "closeAllClients", Qt::QueuedConnection);
}

void NetworkDebuggerView::onBindClicked(const QString& localIp, int localPort,
                                    const QString& remoteIp, int remotePort) {
    auto* worker = channelCtrl_.worker();
    if (!worker) return;

    SPDLOG_INFO("NetworkDebugger: Binding UDP {}:{}", localIp.toStdString(), localPort);
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

    QMetaObject::invokeMethod(worker, "openUdp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, localIp),
                              Q_ARG(int, localPort),
                              Q_ARG(QString, remoteIp),
                              Q_ARG(int, remotePort));
}

void NetworkDebuggerView::onUnbindClicked() {
    auto* worker = channelCtrl_.worker();
    if (!worker) return;

    suppressDisconnectAlert_ = true;
    udpWidget_->setDisplayState(widgets::UdpConnectionWidget::DisplayState::Disconnecting);
    QMetaObject::invokeMethod(worker, "close", Qt::QueuedConnection);
}

void NetworkDebuggerView::onSendRequested(const QByteArray& data) {
    if (!isConnected_) {
        SPDLOG_WARN("NetworkDebuggerView: send rejected (isConnected={}, protocol={}, dataSize={})",
                    isConnected_, static_cast<int>(currentProtocol_), data.size());
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

void NetworkDebuggerView::onWorkerStateChanged(io::ChannelState state, quint64 generation) {
    if (generation != connectionGeneration_) {
        return;
    }

    const bool wasConnected = isConnected_;
    const auto transition = computeTcpStateTransition(
        state,
        wasConnected,
        suppressDisconnectAlert_ || manualDisconnectRequested_);

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
        ? static_cast<widgets::BaseConnectionWidget*>(tcpClientWidget_)
        : static_cast<widgets::BaseConnectionWidget*>(udpWidget_);

    if (!isConnected_ && wasConnected
        && currentProtocol_ == Protocol::TcpClient
        && activeWidget->autoReconnectEnabled()
        // NEW-C: auto-reconnect reacts to passive losses only. A manual
        // disconnect (onDisconnectClicked set the flag) must keep the
        // channel down until the user explicitly connects again.
        && !manualDisconnectRequested_
        && !suppressDisconnectAlert_
        && !channelCtrl_.reconnectTimer()->isActive()) {
        auto& policy = channelCtrl_.reconnectPolicy();
        if (policy.exhausted()) {
            if (monitor_) {
                monitor_->appendInfo(tr("Auto-reconnect exhausted (%1 attempts)")
                                         .arg(policy.maxRetries()));
            }
            channelCtrl_.stopReconnectTimer();
            return;
        }
        const int delay = activeWidget->reconnectDelayMs();
        if (monitor_) {
            monitor_->appendInfo(tr("Auto-reconnect in %1ms (%2)")
                                     .arg(delay)
                                     .arg(policy.statusString()));
        }
        channelCtrl_.startReconnectTimer(delay);
    }

    if (state == io::ChannelState::Open) {
        // A live session invalidates any prior manual-disconnect intent;
        // subsequent losses are passive again (alert + reconnect eligible).
        manualDisconnectRequested_ = false;
        channelCtrl_.reconnectPolicy().onSuccess();
    }
}

void NetworkDebuggerView::onWorkerError(const QString& deviceHint, io::ChannelErrorCode code, const QString& error) {
    if (monitor_) {
        QString localizedMsg;
        switch (code) {
            case io::ChannelErrorCode::ConnectionFailed: localizedMsg = tr("Connection failed"); break;
            case io::ChannelErrorCode::Timeout: localizedMsg = tr("Connection timeout"); break;
            case io::ChannelErrorCode::WriteFailed: localizedMsg = tr("Write failed"); break;
            case io::ChannelErrorCode::ReadFailed: localizedMsg = tr("Read failed"); break;
            case io::ChannelErrorCode::PortNotFound: localizedMsg = tr("Port not found"); break;
            case io::ChannelErrorCode::PermissionDenied: localizedMsg = tr("Permission denied"); break;
            case io::ChannelErrorCode::ConnectionReset: localizedMsg = tr("Connection reset by peer"); break;
            default: localizedMsg = tr("Unknown error"); break;
        }
        monitor_->appendError(tr("Error: %1").arg(localizedMsg));
    }
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("TCP Worker") : deviceHint;
    SPDLOG_ERROR("{} Error (code={}): {}", hint.toStdString(), static_cast<int>(code), error.toStdString());
}

void NetworkDebuggerView::onWorkerMonitor(bool isTx, const QByteArray& data) {
    if (monitor_) {
        monitor_->appendMessage(isTx, data);
    }
}

void NetworkDebuggerView::onServerClientConnected(int clientId, const QString& peerInfo) {
    if (serverClientPanel_) {
        serverClientPanel_->addOrUpdateClient(clientId, peerInfo);
    }
    if (monitor_) {
        monitor_->appendInfo(tr("Client #%1 connected: %2").arg(clientId).arg(peerInfo));
    }
}

void NetworkDebuggerView::onServerClientDisconnected(int clientId) {
    const bool removed = serverClientPanel_ ? serverClientPanel_->removeClient(clientId) : true;
    if (removed && monitor_) {
        monitor_->appendInfo(tr("Client #%1 disconnected").arg(clientId));
    }
}

void NetworkDebuggerView::onServerMonitorWithClient(bool isTx, const QByteArray& data, int clientId) {
    if (monitor_) {
        monitor_->appendMessageWithClient(isTx, data, clientId);
    }
}

void NetworkDebuggerView::onServerStateChanged(io::ChannelState state) {
    // The worker's state contract (see ServerChannelWorker.h) emits only
    // Open and Closed; the Opening/Closing branches below are defensive
    // in case an intermediate listen state is ever introduced.
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

void NetworkDebuggerView::onServerError(const QString& deviceHint, io::ChannelErrorCode code, const QString& error) {
    if (monitor_) {
        QString localizedMsg;
        switch (code) {
            case io::ChannelErrorCode::ConnectionFailed: localizedMsg = tr("Connection failed"); break;
            case io::ChannelErrorCode::Timeout: localizedMsg = tr("Connection timeout"); break;
            case io::ChannelErrorCode::WriteFailed: localizedMsg = tr("Write failed"); break;
            case io::ChannelErrorCode::ReadFailed: localizedMsg = tr("Read failed"); break;
            case io::ChannelErrorCode::PortNotFound: localizedMsg = tr("Port not found"); break;
            case io::ChannelErrorCode::PermissionDenied: localizedMsg = tr("Permission denied"); break;
            case io::ChannelErrorCode::ConnectionReset: localizedMsg = tr("Connection reset by peer"); break;
            default: localizedMsg = tr("Unknown error"); break;
        }
        monitor_->appendError(tr("Server Error: %1").arg(localizedMsg));
    }
    const QString hint = deviceHint.isEmpty() ? QStringLiteral("TCP Server") : deviceHint;
    SPDLOG_ERROR("{} Error (code={}): {}", hint.toStdString(), static_cast<int>(code), error.toStdString());
}

void NetworkDebuggerView::onDisconnectSelectedClientsRequested(const QList<int>& clientIds)
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

void NetworkDebuggerView::onDisconnectAllClientsRequested()
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

void NetworkDebuggerView::retranslateUi() {
    if (inputSection_) inputSection_->setTitle(tr("Send Data"));
    populateProtocolOptions(protocolCombo_);
}

void NetworkDebuggerView::onReconnectTimerTick() {
    auto* activeWidget = tcpClientWidget_;
    auto* worker = channelCtrl_.worker();
    if (!activeWidget || !worker) return;

    if (!activeWidget->autoReconnectEnabled()) {
        channelCtrl_.stopReconnectTimer();
        return;
    }

    if (reconnectHost_.isEmpty()) {
        channelCtrl_.stopReconnectTimer();
        return;
    }

    SPDLOG_INFO("NetworkDebugger: Auto-reconnecting to {}:{} (attempt {})",
                 reconnectHost_.toStdString(), reconnectPort_,
                 channelCtrl_.reconnectPolicy().attemptCount());

    suppressDisconnectAlert_ = false;
    const quint64 generation = ++connectionGeneration_;
    manualDisconnectRequested_ = false; // reconnect tick = fresh intent
    if (monitor_) {
        monitor_->appendInfo(tr("Auto-reconnecting to %1:%2 (attempt %3)...")
                                 .arg(reconnectHost_)
                                 .arg(reconnectPort_)
                                 .arg(channelCtrl_.reconnectPolicy().attemptCount()));
    }
    activeWidget->setDisplayState(widgets::BaseConnectionWidget::DisplayState::Connecting);

    QMetaObject::invokeMethod(worker, "openTcp",
                              Qt::QueuedConnection,
                              Q_ARG(QString, reconnectHost_),
                              Q_ARG(int, reconnectPort_),
                              Q_ARG(quint64, generation));

    channelCtrl_.reconnectTimer()->setInterval(channelCtrl_.reconnectPolicy().nextDelayMs());
    channelCtrl_.reconnectTimer()->start();
}

} // namespace ui::views::network