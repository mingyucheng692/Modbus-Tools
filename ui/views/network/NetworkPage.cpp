// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "NetworkPage.h"
#include "Config.h"
#include "core/common/SettingsKeys.h"
#include "infra/config/ISettingsService.h"
#include "../../widgets/CompactStackedWidget.h"
#include "../../widgets/TcpConnectionWidget.h"
#include "../../widgets/UdpConnectionWidget.h"
#include "../../widgets/ByteMonitorWidget.h"
#include "../../widgets/GenericInputWidget.h"
#include "../../widgets/CollapsibleSection.h"
#include "../../widgets/ServerClientPanel.h"
#include "../../common/ConnectionAlert.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSplitter>
#include <QEvent>
#include <QCoreApplication>

namespace ui::views::network {

namespace {

constexpr auto kTcpClientText = QT_TRANSLATE_NOOP("ui::views::network::Protocol", "TCP Client");
constexpr auto kTcpServerText = QT_TRANSLATE_NOOP("ui::views::network::Protocol", "TCP Server");
constexpr auto kUdpText = QT_TRANSLATE_NOOP("ui::views::network::Protocol", "UDP");

void populateProtocolOptions(QComboBox* combo) {
    if (!combo) return;
    const int currentValue = combo->count() > 0
        ? combo->currentData().toInt()
        : static_cast<int>(NetworkPage::Protocol::TcpClient);
    combo->clear();
    combo->addItem(QCoreApplication::translate("ui::views::network::Protocol", kTcpClientText),
                   static_cast<int>(NetworkPage::Protocol::TcpClient));
    combo->addItem(QCoreApplication::translate("ui::views::network::Protocol", kTcpServerText),
                   static_cast<int>(NetworkPage::Protocol::TcpServer));
    combo->addItem(QCoreApplication::translate("ui::views::network::Protocol", kUdpText),
                   static_cast<int>(NetworkPage::Protocol::Udp));
    const int currentIndex = combo->findData(currentValue);
    combo->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
}

} // namespace

NetworkPage::NetworkPage(infra::config::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService),
      sessionCtrl_(this),
      disconnectAlertHandler_([this]() { ui::common::connection_alert::showDisconnected(this); }) {
    setupUi();
}

NetworkPage::~NetworkPage() noexcept = default;

void NetworkPage::setDisconnectAlertHandler(DisconnectAlertHandler handler) noexcept {
    disconnectAlertHandler_ = std::move(handler);
}

bool NetworkPage::isConnected() const noexcept {
    return sessionCtrl_.state() == io::ChannelState::Open;
}

widgets::BaseConnectionWidget* NetworkPage::activeConnectionWidget() const noexcept {
    switch (currentProtocol_) {
    case Protocol::TcpClient: return tcpClientWidget_;
    case Protocol::TcpServer: return tcpServerWidget_;
    case Protocol::Udp: return udpWidget_;
    }
    return tcpClientWidget_;
}

void NetworkPage::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(4);

    // 1. Protocol section & Connection widgets (Top Bar)
    auto* connRow = new QWidget(this);
    auto* connLayout = new QHBoxLayout(connRow);
    connLayout->setContentsMargins(0, 0, 0, 4);
    connLayout->setSpacing(4);
    connLayout->setAlignment(Qt::AlignTop);

    protocolSection_ = new widgets::CollapsibleSection(settingsService_, connRow);
    protocolSection_->setObjectName(QStringLiteral("networkProtocolSection"));
    protocolSection_->setTitle(tr("Protocol:"));
    protocolSection_->setCollapsible(false);
    protocolSection_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    auto* protoContentLayout = new QHBoxLayout(protocolSection_->contentWidget());
    protoContentLayout->setContentsMargins(4, 0, 4, 0);
    protoContentLayout->setSpacing(2);

    protocolCombo_ = new QComboBox(protocolSection_->contentWidget());
    protocolCombo_->setObjectName(QStringLiteral("networkProtocolCombo"));
    populateProtocolOptions(protocolCombo_);
    protocolCombo_->setFixedWidth(104);
    protoContentLayout->addWidget(protocolCombo_);

    connLayout->addWidget(protocolSection_, 0, Qt::AlignTop);

    connectionStack_ = new widgets::CompactStackedWidget(connRow);
    connectionStack_->setFrameShape(QFrame::NoFrame);
    connectionStack_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    tcpClientWidget_ = new widgets::TcpConnectionWidget(widgets::TcpRole::Client, settingsService_, connectionStack_);
    tcpClientWidget_->setSettingsGroup(QStringLiteral("network/client"));
    tcpClientWidget_->setDefaultPort(config::Network::kDefaultNetworkPort);
    connectionStack_->addWidget(tcpClientWidget_);

    tcpServerWidget_ = new widgets::TcpConnectionWidget(widgets::TcpRole::Server, settingsService_, connectionStack_);
    tcpServerWidget_->setSettingsGroup(QStringLiteral("network/server"));
    tcpServerWidget_->setDefaultPort(config::Network::kDefaultNetworkPort);
    connectionStack_->addWidget(tcpServerWidget_);

    udpWidget_ = new widgets::UdpConnectionWidget(settingsService_, connectionStack_);
    udpWidget_->setSettingsGroup(QStringLiteral("network/udp"));
    udpWidget_->setDefaultPort(config::Network::kDefaultNetworkPort);
    connectionStack_->addWidget(udpWidget_);

    connectionStack_->setCurrentIndex(0);
    connLayout->addWidget(connectionStack_, 1);
    mainLayout->addWidget(connRow);

    // Synchronize collapsible folding across all connection widgets and protocolSection_
    auto syncSectionExpanded = [this](bool expanded) {
        if (protocolSection_) {
            protocolSection_->setExpanded(expanded);
        }
        if (connectionStack_) {
            connectionStack_->updateGeometry();
        }
    };
    connect(tcpClientWidget_, &widgets::BaseConnectionWidget::sectionExpandedChanged,
            this, syncSectionExpanded);
    connect(tcpServerWidget_, &widgets::BaseConnectionWidget::sectionExpandedChanged,
            this, syncSectionExpanded);
    connect(udpWidget_, &widgets::BaseConnectionWidget::sectionExpandedChanged,
            this, syncSectionExpanded);

    // 2. Central Area (Traffic Monitor + Server Client Panel)
    auto* centerSplitter = new QSplitter(Qt::Horizontal, this);
    centerSplitter->setChildrenCollapsible(false);

    monitor_ = new widgets::ByteMonitorWidget(settingsService_, centerSplitter);
    monitor_->setSettingsGroup(QStringLiteral("network/client/traffic"));
    centerSplitter->addWidget(monitor_);

    serverClientPanel_ = new widgets::ServerClientPanel(centerSplitter);
    serverClientPanel_->setObjectName(QStringLiteral("serverClientPanel"));
    serverClientPanel_->hide();
    centerSplitter->addWidget(serverClientPanel_);
    centerSplitter->setStretchFactor(0, 1);
    centerSplitter->setStretchFactor(1, 0);
    mainLayout->addWidget(centerSplitter, 1);

    // 3. Input Section (Bottom)
    inputSection_ = new widgets::CollapsibleSection(settingsService_, this);
    inputSection_->setSettingsKey(QStringLiteral("network/client/ui/inputCollapsed"));
    auto* inputLayout = new QVBoxLayout(inputSection_->contentWidget());
    inputLayout->setContentsMargins(0, 0, 0, 0);

    inputWidget_ = new widgets::GenericInputWidget(settingsService_, inputSection_->contentWidget());
    inputWidget_->setSettingsGroup(QStringLiteral("network/client/input"));
    inputLayout->addWidget(inputWidget_);
    mainLayout->addWidget(inputSection_, 0);

    // Initial snapshot projection
    onSessionSnapshotChanged(sessionCtrl_.currentSnapshot());

    // Protocol selection
    connect(protocolCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NetworkPage::onProtocolChanged);

    // Connection actions
    connect(tcpClientWidget_, &widgets::TcpConnectionWidget::connectClicked,
            this, &NetworkPage::onConnectClicked);
    connect(tcpClientWidget_, &widgets::TcpConnectionWidget::disconnectClicked,
            this, &NetworkPage::onDisconnectClicked);

    connect(tcpServerWidget_, &widgets::TcpConnectionWidget::startListenClicked,
            this, &NetworkPage::onStartListenClicked);
    connect(tcpServerWidget_, &widgets::TcpConnectionWidget::stopListenClicked,
            this, &NetworkPage::onStopListenClicked);

    connect(udpWidget_, &widgets::UdpConnectionWidget::bindClicked,
            this, &NetworkPage::onBindClicked);
    connect(udpWidget_, &widgets::UdpConnectionWidget::unbindClicked,
            this, &NetworkPage::onUnbindClicked);

    // Input actions & auto send
    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested,
            this, &NetworkPage::onSendRequested);
    connect(inputWidget_, &widgets::GenericInputWidget::autoSendToggled,
            this, [this](bool checked, int intervalMs) {
        sessionCtrl_.setAutoSend(checked, intervalMs,
            [this]() { return inputWidget_ ? inputWidget_->getData() : QByteArray(); },
            [this](const QByteArray& data) { onSendRequested(data); });
    });
    connect(inputWidget_, &widgets::GenericInputWidget::autoSendIntervalChanged,
            this, [this](int intervalMs) { sessionCtrl_.setAutoSendInterval(intervalMs); });
    connect(&sessionCtrl_, &application::ChannelSessionController::autoSendStateChanged,
            inputWidget_, &widgets::GenericInputWidget::setAutoSendEnabled);

    // Session controller signals
    connect(&sessionCtrl_, &application::ChannelSessionController::snapshotChanged,
            this, &NetworkPage::onSessionSnapshotChanged);
    connect(&sessionCtrl_, &application::ChannelSessionController::trafficOccurred,
            monitor_, &widgets::ByteMonitorWidget::appendMessage);
    connect(&sessionCtrl_, &application::ChannelSessionController::clientTrafficOccurred,
            monitor_, &widgets::ByteMonitorWidget::appendMessageWithClient);
    connect(&sessionCtrl_, &application::ChannelSessionController::clientConnected,
            this, [this](int clientId, const QString& peerInfo) {
        if (serverClientPanel_) {
            serverClientPanel_->addOrUpdateClient(clientId, peerInfo);
        }
        if (monitor_) {
            monitor_->appendInfo(tr("Client #%1 connected: %2").arg(clientId).arg(peerInfo));
        }
    });
    connect(&sessionCtrl_, &application::ChannelSessionController::clientDisconnected,
            this, [this](int clientId) {
        const bool removed = serverClientPanel_ ? serverClientPanel_->removeClient(clientId) : true;
        if (removed && monitor_) {
            monitor_->appendInfo(tr("Client #%1 disconnected").arg(clientId));
        }
    });
    connect(&sessionCtrl_, &application::ChannelSessionController::errorOccurred,
            this, &NetworkPage::onWorkerError);
    connect(&sessionCtrl_, &application::ChannelSessionController::unexpectedDisconnectDetected,
            this, [this]() {
        emit unexpectedDisconnectDetected();
        if (disconnectAlertHandler_) {
            disconnectAlertHandler_();
        }
    });

    // Server client panel actions
    connect(serverClientPanel_, &widgets::ServerClientPanel::targetSelectionChanged,
            this, [this](bool hasTargets, bool broadcast) {
        sessionCtrl_.setServerTargetState(hasTargets, broadcast);
    });
    connect(serverClientPanel_, &widgets::ServerClientPanel::disconnectClientsRequested,
            this, [this](const QList<int>& clientIds) {
        sessionCtrl_.disconnectClients(clientIds);
    });
    connect(serverClientPanel_, &widgets::ServerClientPanel::disconnectAllClientsRequested,
            this, [this]() {
        sessionCtrl_.disconnectAllClients();
    });

    retranslateUi();

    // Restore persisted protocol
    Protocol initialProtocol = Protocol::TcpClient;
    if (settingsService_ && settingsService_->contains(core::common::settings_keys::kNetworkProtocol)) {
        const int savedVal = settingsService_->value(core::common::settings_keys::kNetworkProtocol).toInt();
        const int foundIndex = protocolCombo_->findData(savedVal);
        if (foundIndex >= 0) {
            protocolCombo_->setCurrentIndex(foundIndex);
            initialProtocol = static_cast<Protocol>(savedVal);
        }
    }
    switchToProtocol(initialProtocol);
}

void NetworkPage::retranslateUi() {
    if (protocolSection_) {
        protocolSection_->setTitle(tr("Protocol:"));
    }
    if (inputSection_) {
        inputSection_->setTitle(tr("Send Data"));
    }
    populateProtocolOptions(protocolCombo_);
}

void NetworkPage::switchToProtocol(Protocol protocol) {
    if (sessionCtrl_.isAutoSendActive()) {
        sessionCtrl_.setAutoSend(false, 0);
    }
    if (inputWidget_) {
        inputWidget_->setAutoSendEnabled(false);
    }
    currentProtocol_ = protocol;

    switch (protocol) {
    case Protocol::TcpClient:
        connectionStack_->setCurrentWidget(tcpClientWidget_);
        monitor_->setSettingsGroup(QStringLiteral("network/client/traffic"));
        inputSection_->setSettingsKey(QStringLiteral("network/client/ui/inputCollapsed"));
        inputWidget_->setSettingsGroup(QStringLiteral("network/client/input"));
        if (serverClientPanel_) {
            serverClientPanel_->clearClients();
            serverClientPanel_->hide();
        }
        sessionCtrl_.setRole(application::ChannelRole::TcpClient);
        break;
    case Protocol::TcpServer:
        connectionStack_->setCurrentWidget(tcpServerWidget_);
        monitor_->setSettingsGroup(QStringLiteral("network/server/traffic"));
        inputSection_->setSettingsKey(QStringLiteral("network/server/ui/inputCollapsed"));
        inputWidget_->setSettingsGroup(QStringLiteral("network/server/input"));
        if (serverClientPanel_) {
            serverClientPanel_->show();
        }
        sessionCtrl_.setRole(application::ChannelRole::TcpServer);
        break;
    case Protocol::Udp:
        connectionStack_->setCurrentWidget(udpWidget_);
        monitor_->setSettingsGroup(QStringLiteral("network/udp/traffic"));
        inputSection_->setSettingsKey(QStringLiteral("network/udp/ui/inputCollapsed"));
        inputWidget_->setSettingsGroup(QStringLiteral("network/udp/input"));
        if (serverClientPanel_) {
            serverClientPanel_->clearClients();
            serverClientPanel_->hide();
        }
        sessionCtrl_.setRole(application::ChannelRole::Udp);
        break;
    }

    if (protocolSection_ && activeConnectionWidget()) {
        protocolSection_->setExpanded(activeConnectionWidget()->isSectionExpanded());
    }
    if (connectionStack_) {
        connectionStack_->updateGeometry();
    }
}

void NetworkPage::onProtocolChanged(int index) {
    if (sessionCtrl_.state() != io::ChannelState::Closed && sessionCtrl_.state() != io::ChannelState::Error) {
        return;
    }
    if (protocolCombo_ && !protocolCombo_->isEnabled()) {
        return;
    }
    currentProtocol_ = static_cast<Protocol>(protocolCombo_->itemData(index).toInt());
    if (settingsService_) {
        settingsService_->setValue(core::common::settings_keys::kNetworkProtocol, static_cast<int>(currentProtocol_));
    }
    switchToProtocol(currentProtocol_);
}

void NetworkPage::onConnectClicked(const QString& ip, int port) {
    if (monitor_) {
        monitor_->appendInfo(tr("Connecting to %1:%2...").arg(ip).arg(port));
    }
    sessionCtrl_.openTcp(ip, port);
}

void NetworkPage::onDisconnectClicked() {
    sessionCtrl_.closeChannel();
}

void NetworkPage::onStartListenClicked(const QString& ip, int port) {
    if (monitor_) {
        monitor_->appendInfo(tr("Starting TCP server on %1:%2...").arg(ip).arg(port));
    }
    sessionCtrl_.openTcpServer(ip, port);
}

void NetworkPage::onStopListenClicked() {
    if (monitor_) {
        monitor_->appendInfo(tr("Stopping TCP server..."));
    }
    sessionCtrl_.closeChannel();
}

void NetworkPage::onBindClicked(const QString& localIp, int localPort,
                                const QString& remoteIp, int remotePort) {
    if (monitor_) {
        if (!remoteIp.isEmpty()) {
            monitor_->appendInfo(tr("Binding UDP %1:%2 -> %3:%4...")
                                     .arg(localIp).arg(localPort)
                                     .arg(remoteIp).arg(remotePort));
        } else {
            monitor_->appendInfo(tr("Binding UDP %1:%2...").arg(localIp).arg(localPort));
        }
    }
    sessionCtrl_.openUdp(localIp, localPort, remoteIp, remotePort);
}

void NetworkPage::onUnbindClicked() {
    sessionCtrl_.closeChannel();
}

void NetworkPage::onWorkerStateChanged(io::ChannelState state, quint64 gen) {
    sessionCtrl_.onWorkerStateChanged(state, gen);
}

void NetworkPage::onSendRequested(const QByteArray& data) {
    if (currentProtocol_ == Protocol::TcpServer) {
        if (!serverClientPanel_) {
            return;
        }
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
        sessionCtrl_.writeToClients(targetClientIds, data);
    } else {
        sessionCtrl_.write(data);
    }
}

void NetworkPage::onSessionSnapshotChanged(const application::SessionUiSnapshot& snap) {
    if (protocolCombo_) {
        protocolCombo_->setEnabled(snap.protocolSelectorEnabled);
    }
    if (auto* active = activeConnectionWidget()) {
        active->applyChannelState(snap.state);
    }
    if (inputWidget_) {
        inputWidget_->setSendControlsEnabled(snap.sendControlsEnabled, snap.sendDisabledTooltip);
    }
    if (snap.stateChanged && monitor_) {
        monitor_->appendInfo(tr("State changed: %1").arg(snap.stateText));
    }
}

void NetworkPage::onWorkerError(const QString& /*deviceHint*/, io::ChannelErrorCode code, const QString& error) {
    if (monitor_) {
        const QString localized = application::ChannelSessionController::formatChannelError(code, error);
        monitor_->appendError(tr("Error: %1").arg(localized));
    }
}

void NetworkPage::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views::network
