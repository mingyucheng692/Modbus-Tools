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

TcpConnectionWidget::TcpConnectionWidget(ui::common::ISettingsService* settingsService, QWidget *parent)
    : QWidget(parent),
      settingsService_(settingsService) {
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
    protocolCombo_->addItem(QStringLiteral("TCP Client"), static_cast<int>(Protocol::TcpClient));
    protocolCombo_->addItem(QStringLiteral("TCP Server"), static_cast<int>(Protocol::TcpServer));
    protocolCombo_->addItem(QStringLiteral("UDP"), static_cast<int>(Protocol::Udp));
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

    autoReconnectCheck_ = new QCheckBox(this);
    layout->addWidget(autoReconnectCheck_);

    reconnectDelaySpin_ = new QSpinBox(this);
    reconnectDelaySpin_->setRange(500, 30000);
    reconnectDelaySpin_->setValue(3000);
    reconnectDelaySpin_->setSuffix(QStringLiteral("ms"));
    reconnectDelaySpin_->setFixedWidth(76);
    layout->addWidget(reconnectDelaySpin_);

    layout->addSpacing(4);

    // Connect/Bind Button
    connectBtn_ = new QPushButton(this);
    connectBtn_->setMinimumWidth(84);
    connectBtn_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    layout->addWidget(connectBtn_);

    // Status
    statusLabel_ = new QLabel(this);
    statusLabel_->setStyleSheet("color: red; font-weight: bold;");
    statusLabel_->setMinimumWidth(96);
    statusLabel_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    layout->addWidget(statusLabel_);

    layout->addStretch();
    mainLayout->addWidget(section_);

    // Connections
    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (isConnected_) {
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
    });

    connect(ipEdit_, &QLineEdit::textChanged, this, &TcpConnectionWidget::saveSettings);
    connect(portEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &TcpConnectionWidget::saveSettings);
    connect(remoteIpEdit_, &QLineEdit::textChanged, this, &TcpConnectionWidget::saveSettings);
    connect(remotePortEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &TcpConnectionWidget::saveSettings);
    connect(autoReconnectCheck_, &QCheckBox::toggled, this, &TcpConnectionWidget::saveSettings);
    connect(autoReconnectCheck_, &QCheckBox::toggled, this, [this]() {
        reconnectDelaySpin_->setVisible(autoReconnectCheck_->isChecked());
    });
    connect(reconnectDelaySpin_, qOverload<int>(&QSpinBox::valueChanged), this, &TcpConnectionWidget::saveSettings);

    loadSettings();
    section_->setSettingsKey(settingsGroup_ + "/ui/connectionSettingsCollapsed");
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
    const QString key = settingsGroup_ + "/port";
    if (!settingsService_ || !settingsService_->contains(key)) {
        portEdit_->setValue(port);
        if (settingsService_) {
            settingsService_->setValue(key, port);
        }
    } else {
        portEdit_->setValue(settingsService_->value(key).toInt());
    }
}

void TcpConnectionWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    if (section_) {
        section_->setSettingsKey(settingsGroup_ + "/ui/connectionSettingsCollapsed");
    }
    loadSettings();
}

void TcpConnectionWidget::loadSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    QSignalBlocker b1(ipEdit_);
    QSignalBlocker b2(portEdit_);
    QSignalBlocker b3(remoteIpEdit_);
    QSignalBlocker b4(remotePortEdit_);
    QSignalBlocker b5(protocolCombo_);
    QSignalBlocker b6(autoReconnectCheck_);
    QSignalBlocker b7(reconnectDelaySpin_);

    const QString ipKey = settingsGroup_ + "/ip";
    const QString portKey = settingsGroup_ + "/port";
    const QString protocolKey = settingsGroup_ + "/protocol";
    const QString remoteIpKey = settingsGroup_ + "/remoteIp";
    const QString remotePortKey = settingsGroup_ + "/remotePort";
    const QString autoReconnectKey = settingsGroup_ + "/autoReconnect";
    const QString reconnectDelayKey = settingsGroup_ + "/reconnectDelay";

    const QString ip = settingsService_->value(ipKey).toString();
    const int port = settingsService_->value(portKey).toInt();
    const int protocolIdx = settingsService_->contains(protocolKey)
        ? settingsService_->value(protocolKey).toInt() : 0;
    const QString remoteIp = settingsService_->value(remoteIpKey).toString();
    const int remotePort = settingsService_->contains(remotePortKey)
        ? settingsService_->value(remotePortKey).toInt() : 0;

    const bool autoReconnect = settingsService_->contains(autoReconnectKey)
        ? settingsService_->value(autoReconnectKey).toBool() : false;
    const int reconnectDelay = settingsService_->contains(reconnectDelayKey)
        ? settingsService_->value(reconnectDelayKey).toInt() : 3000;

    ipEdit_->setText(ip);
    portEdit_->setValue(port);
    protocolCombo_->setCurrentIndex(protocolIdx);
    currentProtocol_ = static_cast<Protocol>(protocolCombo_->currentData().toInt());
    remoteIpEdit_->setText(remoteIp);
    remotePortEdit_->setValue(remotePort);
    autoReconnectCheck_->setChecked(autoReconnect);
    reconnectDelaySpin_->setValue(reconnectDelay);
}

void TcpConnectionWidget::saveSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    settingsService_->setValue(settingsGroup_ + "/ip", ipEdit_->text());
    settingsService_->setValue(settingsGroup_ + "/port", portEdit_->value());
    settingsService_->setValue(settingsGroup_ + "/protocol", protocolCombo_->currentIndex());
    settingsService_->setValue(settingsGroup_ + "/remoteIp", remoteIpEdit_->text());
    settingsService_->setValue(settingsGroup_ + "/remotePort", remotePortEdit_->value());
    settingsService_->setValue(settingsGroup_ + "/autoReconnect", autoReconnectCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + "/reconnectDelay", reconnectDelaySpin_->value());
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

bool TcpConnectionWidget::autoReconnectEnabled() const {
    return autoReconnectCheck_ ? autoReconnectCheck_->isChecked() : false;
}

int TcpConnectionWidget::reconnectDelayMs() const {
    return reconnectDelaySpin_ ? reconnectDelaySpin_->value() : 3000;
}

void TcpConnectionWidget::setConnected(bool connected) {
    isConnected_ = connected;
    if (connected) {
        switch (currentProtocol_) {
        case Protocol::TcpServer:
            connectBtn_->setText(tr("Stop"));
            statusLabel_->setText(tr("Listening"));
            break;
        case Protocol::Udp:
            connectBtn_->setText(tr("Unbind"));
            statusLabel_->setText(tr("Bound"));
            break;
        case Protocol::TcpClient:
            connectBtn_->setText(tr("Disconnect"));
            statusLabel_->setText(tr("Connected"));
            break;
        }
        statusLabel_->setStyleSheet("color: green; font-weight: bold;");
        ipEdit_->setEnabled(false);
        portEdit_->setEnabled(false);
        protocolCombo_->setEnabled(false);
        remoteIpEdit_->setEnabled(false);
        remotePortEdit_->setEnabled(false);
        autoReconnectCheck_->setEnabled(false);
        reconnectDelaySpin_->setEnabled(false);
    } else {
        switch (currentProtocol_) {
        case Protocol::TcpServer:
            connectBtn_->setText(tr("Start Listen"));
            break;
        case Protocol::Udp:
            connectBtn_->setText(tr("Bind"));
            break;
        case Protocol::TcpClient:
            connectBtn_->setText(tr("Connect"));
            break;
        }
        statusLabel_->setText(tr("Disconnected"));
        statusLabel_->setStyleSheet("color: red; font-weight: bold;");
        ipEdit_->setEnabled(true);
        portEdit_->setEnabled(true);
        protocolCombo_->setEnabled(true);
        remoteIpEdit_->setEnabled(true);
        remotePortEdit_->setEnabled(true);
        autoReconnectCheck_->setEnabled(true);
        reconnectDelaySpin_->setEnabled(true);
    }
    updateProtocolUi();
}

void TcpConnectionWidget::updateProtocolUi() {
    if (isConnected_) return;

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
    if (section_) {
        section_->setTitle(tr("Connection Settings"));
    }
    if (remoteIpLabel_) {
        remoteIpLabel_->setText(tr("Remote:"));
    }
    if (remotePortLabel_) {
        remotePortLabel_->setText(tr("Remote Port:"));
    }
    if (autoReconnectCheck_) {
        autoReconnectCheck_->setText(tr("Auto Reconnect"));
    }
    setConnected(isConnected_);
}

void TcpConnectionWidget::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::widgets