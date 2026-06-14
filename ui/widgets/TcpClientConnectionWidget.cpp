/**
 * @file TcpClientConnectionWidget.cpp
 * @brief Implementation of TcpClientConnectionWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TcpClientConnectionWidget.h"
#include "../common/ISettingsService.h"
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>

namespace ui::widgets {

namespace {

bool locksInputs(BaseConnectionWidget::DisplayState state) {
    return state != BaseConnectionWidget::DisplayState::Disconnected;
}

bool usesDisconnectAction(BaseConnectionWidget::DisplayState state) {
    switch (state) {
    case BaseConnectionWidget::DisplayState::Connecting:
    case BaseConnectionWidget::DisplayState::TransportConnected:
    case BaseConnectionWidget::DisplayState::Connected:
        return true;
    case BaseConnectionWidget::DisplayState::Disconnected:
    case BaseConnectionWidget::DisplayState::Disconnecting:
    default:
        return false;
    }
}

} // namespace

TcpClientConnectionWidget::TcpClientConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent)
    : NetworkConnectionWidget(settingsService, parent) {
    setupUi();
}

TcpClientConnectionWidget::~TcpClientConnectionWidget() = default;

void TcpClientConnectionWidget::setupUi() {
    setupCommonUi();
    setupProtocolUi();

    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (usesDisconnectAction(displayState_)) {
            emit disconnectClicked();
        } else {
            saveSettings();
            emit connectClicked(ipEdit_->text(), portEdit_->value());
        }
    });

    loadSettings();
    updateProtocolUi();
    retranslateUi();
}

void TcpClientConnectionWidget::setupProtocolUi() {
    hostLabel_->setText(tr("Host:"));
    connectBtn_->setText(tr("Connect"));
}

void TcpClientConnectionWidget::setConnected(bool connected) {
    if (!connected) {
        setDisplayState(DisplayState::Disconnected);
        return;
    }
    setDisplayState(DisplayState::Connected);
}

void TcpClientConnectionWidget::applyDisplayState() {
    QString buttonText;
    QString statusText;
    QString statusStyle = QStringLiteral("color: red; font-weight: bold;");

    switch (displayState_) {
    case DisplayState::Disconnected:
        buttonText = tr("Connect");
        statusText = tr("Disconnected");
        break;
    case DisplayState::Connecting:
        buttonText = tr("Disconnect");
        statusText = tr("Connecting");
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
        buttonText = tr("Disconnecting");
        statusText = tr("Disconnecting");
        statusStyle = QStringLiteral("color: orange; font-weight: bold;");
        break;
    default:
        break;
    }

    connectBtn_->setText(buttonText);
    statusLabel_->setText(statusText);
    statusLabel_->setStyleSheet(statusStyle);

    const bool inputsEnabled = !locksInputs(displayState_);
    ipEdit_->setEnabled(inputsEnabled);
    portEdit_->setEnabled(inputsEnabled);
    autoReconnectCheck_->setEnabled(inputsEnabled);
    reconnectDelaySpin_->setEnabled(inputsEnabled);
    connectBtn_->setEnabled(displayState_ != DisplayState::Disconnecting);

    updateProtocolUi();
}

void TcpClientConnectionWidget::updateProtocolUi() {
    if (displayState_ != DisplayState::Disconnected) {
        return;
    }
    hostLabel_->setText(tr("Host:"));
    connectBtn_->setText(tr("Connect"));
    autoReconnectCheck_->setVisible(true);
    reconnectDelaySpin_->setVisible(autoReconnectCheck_->isChecked());
}

} // namespace ui::widgets