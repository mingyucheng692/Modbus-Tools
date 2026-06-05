/**
 * @file TcpConnectionWidget.cpp
 * @brief Implementation of TcpConnectionWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TcpConnectionWidget.h"
#include "AppConstants.h"
#include "CollapsibleSection.h"
#include "../common/ISettingsService.h"
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QEvent>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace ui::widgets {

namespace {

constexpr auto kTcpConnectionContext = "ui::widgets::TcpConnectionWidget";
constexpr auto kTcpClientText = QT_TRANSLATE_NOOP("ui::widgets::TcpConnectionWidget", "TCP Client");
constexpr auto kTcpServerText = QT_TRANSLATE_NOOP("ui::widgets::TcpConnectionWidget", "TCP Server");
constexpr auto kUdpText = QT_TRANSLATE_NOOP("ui::widgets::TcpConnectionWidget", "UDP");

QString trTcpConnection(const char* sourceText)
{
    return QCoreApplication::translate(kTcpConnectionContext, sourceText);
}

void repopulateProtocolOptions(QComboBox* combo)
{
    if (!combo) {
        return;
    }

    const int currentValue = combo->count() > 0
        ? combo->currentData().toInt()
        : static_cast<int>(TcpConnectionWidget::Protocol::TcpClient);
    QSignalBlocker blocker(combo);
    combo->clear();
    combo->addItem(trTcpConnection(kTcpClientText), static_cast<int>(TcpConnectionWidget::Protocol::TcpClient));
    combo->addItem(trTcpConnection(kTcpServerText), static_cast<int>(TcpConnectionWidget::Protocol::TcpServer));
    combo->addItem(trTcpConnection(kUdpText), static_cast<int>(TcpConnectionWidget::Protocol::Udp));

    const int currentIndex = combo->findData(currentValue);
    combo->setCurrentIndex(currentIndex >= 0 ? currentIndex : 0);
}

bool locksInputs(TcpConnectionWidget::DisplayState state)
{
    return state != TcpConnectionWidget::DisplayState::Disconnected;
}

bool usesDisconnectAction(TcpConnectionWidget::DisplayState state)
{
    switch (state) {
    case TcpConnectionWidget::DisplayState::Connecting:
    case TcpConnectionWidget::DisplayState::TransportConnected:
    case TcpConnectionWidget::DisplayState::Connected:
    case TcpConnectionWidget::DisplayState::Listening:
    case TcpConnectionWidget::DisplayState::Bound:
        return true;
    case TcpConnectionWidget::DisplayState::Disconnected:
    case TcpConnectionWidget::DisplayState::Disconnecting:
        return false;
    default:
        return false;
    }
}

} // namespace

TcpConnectionWidget::TcpConnectionWidget(ui::common::ISettingsService* settingsService, QWidget *parent)
    : BaseConnectionWidget(settingsService, parent) {
    settingsGroup_ = QStringLiteral("modbus/tcp");
    setupUi();
}

TcpConnectionWidget::~TcpConnectionWidget() = default;

void TcpConnectionWidget::setupUi() {
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    section_ = new CollapsibleSection(settingsService_, this);
    auto layout = new QHBoxLayout(section_->contentWidget());
    layout->setContentsMargins(4, 0, 4, 0);
    layout->setSpacing(2);

    // Protocol ComboBox
    protocolCombo_ = new QComboBox(this);
    repopulateProtocolOptions(protocolCombo_);
    protocolCombo_->setFixedWidth(104);
    layout->addWidget(protocolCombo_);

    // IP Address
    hostLabel_ = new QLabel(this);
    layout->addWidget(hostLabel_);
    ipEdit_ = new QLineEdit(this);
    ipEdit_->setMinimumWidth(112);
    ipEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(ipEdit_);

    // Port
    portLabel_ = new QLabel(this);
    layout->addWidget(portLabel_);
    portEdit_ = new QSpinBox(this);
    portEdit_->setRange(app::constants::Values::Network::kMinTcpPort,
                        app::constants::Values::Network::kMaxTcpPort);
    portEdit_->setValue(app::constants::Values::Network::kDefaultModbusTcpPort);
    portEdit_->setFixedWidth(76);
    layout->addWidget(portEdit_);

    // Remote IP (UDP only)
    remoteIpLabel_ = new QLabel(this);
    layout->addWidget(remoteIpLabel_);
    remoteIpEdit_ = new QLineEdit(this);
    remoteIpEdit_->setMinimumWidth(112);
    remoteIpEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    remoteIpEdit_->setVisible(false);
    layout->addWidget(remoteIpEdit_);

    // Remote Port (UDP only)
    remotePortLabel_ = new QLabel(this);
    layout->addWidget(remotePortLabel_);
    remotePortEdit_ = new QSpinBox(this);
    remotePortEdit_->setRange(app::constants::Values::Network::kMinTcpPort,
                              app::constants::Values::Network::kMaxTcpPort);
    remotePortEdit_->setValue(0);
    remotePortEdit_->setFixedWidth(76);
    remotePortEdit_->setVisible(false);
    remotePortEdit_->setSpecialValueText(QStringLiteral("-"));
    layout->addWidget(remotePortEdit_);

    // Create common widgets (autoReconnectCheck_, reconnectDelaySpin_, connectBtn_, statusLabel_)
    createCommonWidgets(section_->contentWidget());

    layout->addWidget(autoReconnectCheck_);
    layout->addWidget(reconnectDelaySpin_);
    layout->addSpacing(4);
    layout->addWidget(connectBtn_);
    layout->addWidget(statusLabel_);

    layout->addStretch();
    mainLayout->addWidget(section_);

    // Connections
    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (usesDisconnectAction(displayState_)) {
            switch (currentProtocol_) {
            case Protocol::TcpServer:
                emit stopListenClicked();
                break;
            case Protocol::Udp:
                emit unbindClicked();
                break;
            case Protocol::TcpClient:
                emit disconnectClicked();
                break;
            }
        } else {
            saveSettings();
            switch (currentProtocol_) {
            case Protocol::TcpServer:
                emit startListenClicked(ipEdit_->text(), portEdit_->value());
                break;
            case Protocol::Udp:
                emit bindClicked(ipEdit_->text(), portEdit_->value(),
                                 remoteIpEdit_->text(), remotePortEdit_->value());
                break;
            case Protocol::TcpClient:
                emit connectClicked(ipEdit_->text(), portEdit_->value());
                break;
            }
        }
    });

    connect(protocolCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        Q_UNUSED(idx);
        currentProtocol_ = static_cast<Protocol>(protocolCombo_->currentData().toInt());
        saveSettings();
        updateProtocolUi();
        emit protocolChanged(currentProtocol_);
    });

    connect(ipEdit_, &QLineEdit::textChanged, this, &TcpConnectionWidget::saveSettings);
    connect(portEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &TcpConnectionWidget::saveSettings);
    connect(remoteIpEdit_, &QLineEdit::textChanged, this, &TcpConnectionWidget::saveSettings);
    connect(remotePortEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &TcpConnectionWidget::saveSettings);

    setupCommonConnections();

    loadSettings();
    section_->setSettingsKey(settingsGroup_ + QStringLiteral("/ui/connectionSettingsCollapsed"));
    updateProtocolUi();
    retranslateUi();
}

void TcpConnectionWidget::setDefaultPort(int port) {
    defaultPort_ = port;
    if (!portEdit_) return;
    if (settingsGroup_.isEmpty()) {
        portEdit_->setValue(port);
        return;
    }
    const QString key = settingsGroup_ + QStringLiteral("/port");
    if (!settingsService_ || !settingsService_->contains(key)) {
        portEdit_->setValue(port);
        if (settingsService_) {
            settingsService_->setValue(key, port);
        }
    } else {
        portEdit_->setValue(settingsService_->value(key).toInt());
    }
}

void TcpConnectionWidget::loadSettings() {
    loadCommonSettings();
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    QSignalBlocker b1(ipEdit_);
    QSignalBlocker b2(portEdit_);
    QSignalBlocker b3(remoteIpEdit_);
    QSignalBlocker b4(remotePortEdit_);
    QSignalBlocker b5(protocolCombo_);

    const QString ipKey = settingsGroup_ + QStringLiteral("/ip");
    const QString portKey = settingsGroup_ + QStringLiteral("/port");
    const QString protocolKey = settingsGroup_ + QStringLiteral("/protocol");
    const QString remoteIpKey = settingsGroup_ + QStringLiteral("/remoteIp");
    const QString remotePortKey = settingsGroup_ + QStringLiteral("/remotePort");

    const QString ip = settingsService_->value(ipKey).toString();
    const int port = settingsService_->value(portKey).toInt();
    const int protocolIdx = settingsService_->contains(protocolKey)
        ? settingsService_->value(protocolKey).toInt() : 0;
    const QString remoteIp = settingsService_->value(remoteIpKey).toString();
    const int remotePort = settingsService_->contains(remotePortKey)
        ? settingsService_->value(remotePortKey).toInt() : 0;

    ipEdit_->setText(ip);
    portEdit_->setValue(port);
    protocolCombo_->setCurrentIndex(protocolIdx);
    currentProtocol_ = static_cast<Protocol>(protocolCombo_->currentData().toInt());
    remoteIpEdit_->setText(remoteIp);
    remotePortEdit_->setValue(remotePort);
}

void TcpConnectionWidget::saveSettings() {
    saveCommonSettings();
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    settingsService_->setValue(settingsGroup_ + QStringLiteral("/ip"), ipEdit_->text());
    settingsService_->setValue(settingsGroup_ + QStringLiteral("/port"), portEdit_->value());
    settingsService_->setValue(settingsGroup_ + QStringLiteral("/protocol"), protocolCombo_->currentIndex());
    settingsService_->setValue(settingsGroup_ + QStringLiteral("/remoteIp"), remoteIpEdit_->text());
    settingsService_->setValue(settingsGroup_ + QStringLiteral("/remotePort"), remotePortEdit_->value());
}

QString TcpConnectionWidget::getIpAddress() const {
    return ipEdit_->text();
}

int TcpConnectionWidget::getPort() const {
    return portEdit_->value();
}

TcpConnectionWidget::Protocol TcpConnectionWidget::currentProtocol() const {
    return currentProtocol_;
}

void TcpConnectionWidget::setConnected(bool connected) {
    if (!connected) {
        setDisplayState(DisplayState::Disconnected);
        return;
    }

    switch (currentProtocol_) {
    case Protocol::TcpServer:
        setDisplayState(DisplayState::Listening);
        break;
    case Protocol::Udp:
        setDisplayState(DisplayState::Bound);
        break;
    case Protocol::TcpClient:
        setDisplayState(DisplayState::Connected);
        break;
    }
}

void TcpConnectionWidget::applyDisplayState() {
    QString buttonText;
    QString statusText;
    QString statusStyle = QStringLiteral("color: red; font-weight: bold;");

    switch (displayState_) {
    case DisplayState::Disconnected:
        switch (currentProtocol_) {
        case Protocol::TcpServer:
            buttonText = tr("Start Listen");
            break;
        case Protocol::Udp:
            buttonText = tr("Bind");
            break;
        case Protocol::TcpClient:
            buttonText = tr("Connect");
            break;
        }
        statusText = tr("Disconnected");
        break;
    case DisplayState::Connecting:
        switch (currentProtocol_) {
        case Protocol::TcpServer:
            buttonText = tr("Stop");
            statusText = tr("Starting");
            break;
        case Protocol::Udp:
            buttonText = tr("Unbind");
            statusText = tr("Binding");
            break;
        case Protocol::TcpClient:
            buttonText = tr("Disconnect");
            statusText = tr("Connecting");
            break;
        }
        statusStyle = QStringLiteral("color: orange; font-weight: bold;");
        break;
    case DisplayState::TransportConnected:
        buttonText = tr("Disconnect");
        statusText = tr("Transport Connected");
        statusStyle = QStringLiteral("color: #1f6feb; font-weight: bold;");
        break;
    case DisplayState::Connected:
        buttonText = tr("Disconnect");
        statusText = tr("Connected");
        statusStyle = QStringLiteral("color: green; font-weight: bold;");
        break;
    case DisplayState::Disconnecting:
        switch (currentProtocol_) {
        case Protocol::TcpServer:
            buttonText = tr("Stopping");
            break;
        case Protocol::Udp:
            buttonText = tr("Unbinding");
            break;
        case Protocol::TcpClient:
            buttonText = tr("Disconnecting");
            break;
        }
        statusText = tr("Disconnecting");
        statusStyle = QStringLiteral("color: orange; font-weight: bold;");
        break;
    case DisplayState::Listening:
        buttonText = tr("Stop");
        statusText = tr("Listening");
        statusStyle = QStringLiteral("color: green; font-weight: bold;");
        break;
    case DisplayState::Bound:
        buttonText = tr("Unbind");
        statusText = tr("Bound");
        statusStyle = QStringLiteral("color: green; font-weight: bold;");
        break;
    }

    connectBtn_->setText(buttonText);
    statusLabel_->setText(statusText);
    statusLabel_->setStyleSheet(statusStyle);

    const bool inputsEnabled = !locksInputs(displayState_);
    ipEdit_->setEnabled(inputsEnabled);
    portEdit_->setEnabled(inputsEnabled);
    protocolCombo_->setEnabled(inputsEnabled);
    remoteIpEdit_->setEnabled(inputsEnabled);
    remotePortEdit_->setEnabled(inputsEnabled);
    autoReconnectCheck_->setEnabled(inputsEnabled);
    reconnectDelaySpin_->setEnabled(inputsEnabled);
    connectBtn_->setEnabled(displayState_ != DisplayState::Disconnecting);

    updateProtocolUi();
}

void TcpConnectionWidget::updateProtocolUi() {
    if (displayState_ != DisplayState::Disconnected) {
        return;
    }

    switch (currentProtocol_) {
    case Protocol::TcpClient:
        hostLabel_->setText(tr("Host:"));
        portLabel_->setText(tr("Port:"));
        connectBtn_->setText(tr("Connect"));
        remoteIpLabel_->setVisible(false);
        remoteIpEdit_->setVisible(false);
        remotePortLabel_->setVisible(false);
        remotePortEdit_->setVisible(false);
        autoReconnectCheck_->setVisible(true);
        reconnectDelaySpin_->setVisible(autoReconnectCheck_->isChecked());
        break;
    case Protocol::TcpServer:
        hostLabel_->setText(tr("Listen:"));
        portLabel_->setText(tr("Port:"));
        connectBtn_->setText(tr("Start Listen"));
        remoteIpLabel_->setVisible(false);
        remoteIpEdit_->setVisible(false);
        remotePortLabel_->setVisible(false);
        remotePortEdit_->setVisible(false);
        autoReconnectCheck_->setVisible(false);
        reconnectDelaySpin_->setVisible(false);
        break;
    case Protocol::Udp:
        hostLabel_->setText(tr("Local:"));
        portLabel_->setText(tr("Port:"));
        connectBtn_->setText(tr("Bind"));
        remoteIpLabel_->setVisible(true);
        remoteIpEdit_->setVisible(true);
        remotePortLabel_->setVisible(true);
        remotePortEdit_->setVisible(true);
        autoReconnectCheck_->setVisible(false);
        reconnectDelaySpin_->setVisible(false);
        break;
    }
}

void TcpConnectionWidget::retranslateUi() {
    retranslateCommonUi();
    repopulateProtocolOptions(protocolCombo_);
    if (remoteIpLabel_) {
        remoteIpLabel_->setText(tr("Remote:"));
    }
    if (remotePortLabel_) {
        remotePortLabel_->setText(tr("Remote Port:"));
    }
    applyDisplayState();
}

void TcpConnectionWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets
