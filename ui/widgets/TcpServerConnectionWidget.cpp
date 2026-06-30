/**
 * @file TcpServerConnectionWidget.cpp
 * @brief Implementation of TcpServerConnectionWidget — lean subclass providing
 *        only protocol-specific display data and button wiring.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TcpServerConnectionWidget.h"
#include "../common/ISettingsService.h"
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

namespace ui::widgets {

TcpServerConnectionWidget::TcpServerConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent)
    : NetworkConnectionWidget(settingsService, parent) {
    setupNetworkUi();
}

TcpServerConnectionWidget::~TcpServerConnectionWidget() = default;

// ---- Protocol-specific UI ----

void TcpServerConnectionWidget::setupProtocolUi() {
    hostLabel_->setText(tr("Listen:"));
    connectBtn_->setText(tr("Start Listen"));
    autoReconnectCheck_->setVisible(false);
    reconnectDelaySpin_->setVisible(false);
}

void TcpServerConnectionWidget::setupButtonConnection() {
    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (isActiveState(displayState_)) {
            emit stopListenClicked();
        } else {
            saveSettings();
            emit startListenClicked(ipEdit_->text(), portEdit_->value());
        }
    });
}

// ---- Display State Data ----

StateDisplayInfo TcpServerConnectionWidget::getStateDisplayInfo(DisplayState state) const {
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

bool TcpServerConnectionWidget::isActiveState(DisplayState state) const {
    switch (state) {
    case DisplayState::Connecting:
    case DisplayState::Listening:
        return true;
    default:
        return false;
    }
}

NetworkConnectionWidget::DisplayState TcpServerConnectionWidget::connectedState() const {
    return DisplayState::Listening;
}

void TcpServerConnectionWidget::updateProtocolUi() {
    if (displayState_ != DisplayState::Disconnected) return;
    hostLabel_->setText(tr("Listen:"));
    connectBtn_->setText(tr("Start Listen"));
}

} // namespace ui::widgets
