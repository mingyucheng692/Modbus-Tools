/**
 * @file TcpServerConnectionWidget.cpp
 * @brief Implementation of TcpServerConnectionWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TcpServerConnectionWidget.h"
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
    case BaseConnectionWidget::DisplayState::Listening:
        return true;
    case BaseConnectionWidget::DisplayState::Disconnected:
    case BaseConnectionWidget::DisplayState::Disconnecting:
    default:
        return false;
    }
}

} // namespace

TcpServerConnectionWidget::TcpServerConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent)
    : NetworkConnectionWidget(settingsService, parent) {
    setupUi();
}

TcpServerConnectionWidget::~TcpServerConnectionWidget() = default;

void TcpServerConnectionWidget::setupUi() {
    setupCommonUi();
    setupProtocolUi();

    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (usesDisconnectAction(displayState_)) {
            emit stopListenClicked();
        } else {
            saveSettings();
            emit startListenClicked(ipEdit_->text(), portEdit_->value());
        }
    });

    loadSettings();
    updateProtocolUi();
    retranslateUi();
}

void TcpServerConnectionWidget::setupProtocolUi() {
    hostLabel_->setText(tr("Listen:"));
    connectBtn_->setText(tr("Start Listen"));
    autoReconnectCheck_->setVisible(false);
    reconnectDelaySpin_->setVisible(false);
}

void TcpServerConnectionWidget::setConnected(bool connected) {
    if (!connected) {
        setDisplayState(DisplayState::Disconnected);
        return;
    }
    setDisplayState(DisplayState::Listening);
}

void TcpServerConnectionWidget::applyDisplayState() {
    QString buttonText;
    QString statusText;
    QString statusStyle = QStringLiteral("color: red; font-weight: bold;");

    switch (displayState_) {
    case DisplayState::Disconnected:
        buttonText = tr("Start Listen");
        statusText = tr("Disconnected");
        break;
    case DisplayState::Connecting:
        buttonText = tr("Stop");
        statusText = tr("Starting");
        statusStyle = QStringLiteral("color: orange; font-weight: bold;");
        break;
    case DisplayState::Listening:
        buttonText = tr("Stop");
        statusText = tr("Listening");
        statusStyle = QStringLiteral("color: green; font-weight: bold;");
        break;
    case DisplayState::Disconnecting:
        buttonText = tr("Stopping");
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
    connectBtn_->setEnabled(displayState_ != DisplayState::Disconnecting);

    updateProtocolUi();
}

void TcpServerConnectionWidget::updateProtocolUi() {
    if (displayState_ != DisplayState::Disconnected) {
        return;
    }
    hostLabel_->setText(tr("Listen:"));
    connectBtn_->setText(tr("Start Listen"));
}

} // namespace ui::widgets