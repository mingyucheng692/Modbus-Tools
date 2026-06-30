/**
 * @file TcpClientConnectionWidget.cpp
 * @brief Implementation of TcpClientConnectionWidget — lean subclass providing
 *        only protocol-specific display data and button wiring.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TcpClientConnectionWidget.h"
#include "../common/ISettingsService.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

namespace ui::widgets {

TcpClientConnectionWidget::TcpClientConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent)
    : NetworkConnectionWidget(settingsService, parent) {
    setupNetworkUi();
}

TcpClientConnectionWidget::~TcpClientConnectionWidget() = default;

// ---- Protocol-specific UI ----

void TcpClientConnectionWidget::setupProtocolUi() {
    hostLabel_->setText(tr("Host:"));
    connectBtn_->setText(tr("Connect"));
}

void TcpClientConnectionWidget::setupButtonConnection() {
    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (isActiveState(displayState_)) {
            emit disconnectClicked();
        } else {
            saveSettings();
            emit connectClicked(ipEdit_->text(), portEdit_->value());
        }
    });
}

// ---- Display State Data ----

StateDisplayInfo TcpClientConnectionWidget::getStateDisplayInfo(DisplayState state) const {
    switch (state) {
    case DisplayState::Disconnected:
        return {tr("Connect"), tr("Disconnected"), QStringLiteral("color: red; font-weight: bold;")};
    case DisplayState::Connecting:
        return {tr("Disconnect"), tr("Connecting"), QStringLiteral("color: orange; font-weight: bold;")};
    case DisplayState::TransportConnected:
        return {tr("Disconnect"), tr("Transport Connected"), QStringLiteral("color: #1f6feb; font-weight: bold;")};
    case DisplayState::Connected:
        return {tr("Disconnect"), tr("Connected"), QStringLiteral("color: green; font-weight: bold;")};
    case DisplayState::Disconnecting:
        return {tr("Disconnecting"), tr("Disconnecting"), QStringLiteral("color: orange; font-weight: bold;")};
    default:
        return {};
    }
}

bool TcpClientConnectionWidget::isActiveState(DisplayState state) const {
    switch (state) {
    case DisplayState::Connecting:
    case DisplayState::TransportConnected:
    case DisplayState::Connected:
        return true;
    default:
        return false;
    }
}

NetworkConnectionWidget::DisplayState TcpClientConnectionWidget::connectedState() const {
    return DisplayState::Connected;
}

void TcpClientConnectionWidget::updateProtocolUi() {
    if (displayState_ != DisplayState::Disconnected) return;
    hostLabel_->setText(tr("Host:"));
    connectBtn_->setText(tr("Connect"));
    autoReconnectCheck_->setVisible(true);
    reconnectDelaySpin_->setVisible(autoReconnectCheck_->isChecked());
}

} // namespace ui::widgets
