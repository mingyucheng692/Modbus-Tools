/**
 * @file TcpConnectionWidget.cpp
 * @brief Implementation of TcpConnectionWidget — unified TCP Client/Server widget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TcpConnectionWidget.h"
#include "../../core/common/ISettingsService.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

namespace ui::widgets {

TcpConnectionWidget::TcpConnectionWidget(TcpRole role, core::common::ISettingsService* settingsService, QWidget* parent)
    : NetworkConnectionWidget(settingsService, parent),
      role_(role) {
    setupNetworkUi();
}

TcpConnectionWidget::~TcpConnectionWidget() = default;

// ---- Protocol-specific UI ----

void TcpConnectionWidget::setupProtocolUi() {
    if (role_ == TcpRole::Client) {
        hostLabel_->setText(tr("Host:"));
        connectBtn_->setText(tr("Connect"));
    } else {
        hostLabel_->setText(tr("Listen:"));
        connectBtn_->setText(tr("Start Listen"));
        autoReconnectCheck_->setVisible(false);
        reconnectDelaySpin_->setVisible(false);
    }
}

void TcpConnectionWidget::setupButtonConnection() {
    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (isActiveState(displayState_)) {
            if (role_ == TcpRole::Client) {
                emit disconnectClicked();
            } else {
                emit stopListenClicked();
            }
        } else {
            saveSettings();
            if (role_ == TcpRole::Client) {
                emit connectClicked(ipEdit_->text(), portEdit_->value());
            } else {
                emit startListenClicked(ipEdit_->text(), portEdit_->value());
            }
        }
    });
}

// ---- Display State Data ----

StateDisplayInfo TcpConnectionWidget::getStateDisplayInfo(DisplayState state) const {
    if (role_ == TcpRole::Client) {
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
    } else {
        switch (state) {
        case DisplayState::Disconnected:
            return {tr("Start Listen"), tr("Disconnected"), QStringLiteral("color: red; font-weight: bold;")};
        case DisplayState::Connecting:
            return {tr("Stop"), tr("Starting"), QStringLiteral("color: orange; font-weight: bold;")};
        case DisplayState::Listening:
            return {tr("Stop"), tr("Listening"), QStringLiteral("color: green; font-weight: bold;")};
        case DisplayState::Disconnecting:
            return {tr("Stopping"), tr("Disconnecting"), QStringLiteral("color: orange; font-weight: bold;")};
        default:
            return {};
        }
    }
}

bool TcpConnectionWidget::isActiveState(DisplayState state) const {
    if (role_ == TcpRole::Client) {
        switch (state) {
        case DisplayState::Connecting:
        case DisplayState::TransportConnected:
        case DisplayState::Connected:
            return true;
        default:
            return false;
        }
    } else {
        switch (state) {
        case DisplayState::Connecting:
        case DisplayState::Listening:
            return true;
        default:
            return false;
        }
    }
}

NetworkConnectionWidget::DisplayState TcpConnectionWidget::connectedState() const {
    return (role_ == TcpRole::Client) ? DisplayState::Connected : DisplayState::Listening;
}

void TcpConnectionWidget::updateProtocolUi() {
    if (displayState_ != DisplayState::Disconnected) return;

    if (role_ == TcpRole::Client) {
        hostLabel_->setText(tr("Host:"));
        connectBtn_->setText(tr("Connect"));
        autoReconnectCheck_->setVisible(true);
        reconnectDelaySpin_->setVisible(autoReconnectCheck_->isChecked());
    } else {
        hostLabel_->setText(tr("Listen:"));
        connectBtn_->setText(tr("Start Listen"));
    }
}

} // namespace ui::widgets