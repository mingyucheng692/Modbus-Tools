/**
 * @file NetworkConnectionWidget.cpp
 * @brief Implementation of NetworkConnectionWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "NetworkConnectionWidget.h"
#include "CollapsibleSection.h"
#include "common/SettingsKeys.h"
#include "infra/config/ISettingsService.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>


namespace ui::widgets {

NetworkConnectionWidget::NetworkConnectionWidget(infra::config::ISettingsService* settingsService, QWidget* parent)
    : BaseConnectionWidget(settingsService, parent) {
}

NetworkConnectionWidget::~NetworkConnectionWidget() = default;

void NetworkConnectionWidget::setupCommonUi() {
    auto* layout = setupBaseUi();

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
    portEdit_->setRange(config::Network::kMinTcpPort,
                        config::Network::kMaxTcpPort);
    portEdit_->setValue(defaultPort_);
    portEdit_->setFixedWidth(76);
    layout->addWidget(portEdit_);

    // Common widgets (autoReconnect, connectBtn, statusLabel)
    createCommonWidgets(section_->contentWidget());

    layout->addWidget(autoReconnectCheck_);
    layout->addWidget(reconnectDelaySpin_);
    layout->addSpacing(4);
    layout->addWidget(connectBtn_);
    layout->addWidget(statusLabel_);

    layout->addStretch();
    auto* mainLayout = qobject_cast<QHBoxLayout*>(this->layout());
    if (mainLayout) {
        mainLayout->addWidget(section_);
    }

    setupCommonConnections();

    connect(ipEdit_, &QLineEdit::textChanged, this, &NetworkConnectionWidget::saveSettings);
    connect(portEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &NetworkConnectionWidget::saveSettings);
}

void NetworkConnectionWidget::setDefaultPort(int port) {
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

QString NetworkConnectionWidget::getIpAddress() const {
    return ipEdit_->text();
}

int NetworkConnectionWidget::getPort() const {
    return portEdit_->value();
}

void NetworkConnectionWidget::loadSettings() {
    loadCommonSettings();
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    QSignalBlocker b1(ipEdit_);
    QSignalBlocker b2(portEdit_);

    const QString ipKey = settingsGroup_ + QStringLiteral("/ip");
    const QString portKey = settingsGroup_ + QStringLiteral("/port");

    ipEdit_->setText(settingsService_->value(ipKey).toString());
    portEdit_->setValue(settingsService_->value(portKey).toInt());
}

void NetworkConnectionWidget::saveSettings() {
    saveCommonSettings();
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    settingsService_->setValue(settingsGroup_ + QStringLiteral("/ip"), ipEdit_->text());
    settingsService_->setValue(settingsGroup_ + QStringLiteral("/port"), portEdit_->value());
}

void NetworkConnectionWidget::retranslateUi() {
    BaseConnectionWidget::retranslateUi();
    if (portLabel_) {
        portLabel_->setText(tr("Port:"));
    }
}

// ---- Template Methods ----

void NetworkConnectionWidget::setupNetworkUi() {
    setupCommonUi();
    setupProtocolUi();
    setupButtonConnection();
    loadSettings();
    updateProtocolUi();
    retranslateUi();
}

void NetworkConnectionWidget::setConnected(bool connected) {
    if (!connected) {
        setDisplayState(DisplayState::Disconnected);
        return;
    }
    setDisplayState(connectedState());
}

void NetworkConnectionWidget::applyInputWidgetsState(bool enabled) {
    // Network-specific input widgets only; autoReconnectCheck_ /
    // reconnectDelaySpin_ / connectBtn_ are handled by the base skeleton.
    ipEdit_->setEnabled(enabled);
    portEdit_->setEnabled(enabled);
}

} // namespace ui::widgets