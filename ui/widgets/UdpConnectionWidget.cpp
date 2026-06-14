/**
 * @file UdpConnectionWidget.cpp
 * @brief Implementation of UdpConnectionWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UdpConnectionWidget.h"
#include "AppConstants.h"
#include "CollapsibleSection.h"
#include "../common/ISettingsService.h"
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>

namespace ui::widgets {

namespace {

bool locksInputs(BaseConnectionWidget::DisplayState state) {
    return state != BaseConnectionWidget::DisplayState::Disconnected;
}

bool usesDisconnectAction(BaseConnectionWidget::DisplayState state) {
    switch (state) {
    case BaseConnectionWidget::DisplayState::Connecting:
    case BaseConnectionWidget::DisplayState::Bound:
        return true;
    case BaseConnectionWidget::DisplayState::Disconnected:
    case BaseConnectionWidget::DisplayState::Disconnecting:
    default:
        return false;
    }
}

} // namespace

UdpConnectionWidget::UdpConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent)
    : NetworkConnectionWidget(settingsService, parent) {
    setupUi();
}

UdpConnectionWidget::~UdpConnectionWidget() = default;

void UdpConnectionWidget::setupUi() {
    setupCommonUi();

    // Insert remote IP/Port before the common widgets (autoReconnect, etc.)
    auto* layout = section_->contentWidget()->layout();
    if (layout) {
        remoteIpLabel_ = new QLabel(this);
        remoteIpEdit_ = new QLineEdit(this);
        remoteIpEdit_->setMinimumWidth(112);
        remoteIpEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        remotePortLabel_ = new QLabel(this);
        remotePortEdit_ = new QSpinBox(this);
        remotePortEdit_->setRange(app::constants::Values::Network::kMinTcpPort,
                                  app::constants::Values::Network::kMaxTcpPort);
        remotePortEdit_->setValue(0);
        remotePortEdit_->setFixedWidth(76);
        remotePortEdit_->setSpecialValueText(QStringLiteral("-"));

        // Insert after portEdit_ (index 4 in layout: hostLabel=0, ipEdit=1, portLabel=2, portEdit=3)
        // Find portEdit_ index and insert after it
        auto* boxLayout = qobject_cast<QBoxLayout*>(layout);
        if (boxLayout) {
            int portIdx = boxLayout->indexOf(portEdit_);
            if (portIdx >= 0) {
                boxLayout->insertWidget(portIdx + 1, remoteIpLabel_);
                boxLayout->insertWidget(portIdx + 2, remoteIpEdit_);
                boxLayout->insertWidget(portIdx + 3, remotePortLabel_);
                boxLayout->insertWidget(portIdx + 4, remotePortEdit_);
            }
        }
    }

    setupProtocolUi();

    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (usesDisconnectAction(displayState_)) {
            emit unbindClicked();
        } else {
            saveSettings();
            emit bindClicked(ipEdit_->text(), portEdit_->value(),
                             remoteIpEdit_->text(), remotePortEdit_->value());
        }
    });

    connect(remoteIpEdit_, &QLineEdit::textChanged, this, &UdpConnectionWidget::saveSettings);
    connect(remotePortEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &UdpConnectionWidget::saveSettings);

    loadSettings();
    updateProtocolUi();
    retranslateUi();
}

void UdpConnectionWidget::setupProtocolUi() {
    hostLabel_->setText(tr("Local:"));
    connectBtn_->setText(tr("Bind"));
    remoteIpLabel_->setText(tr("Remote:"));
    remotePortLabel_->setText(tr("Remote Port:"));
    autoReconnectCheck_->setVisible(false);
    reconnectDelaySpin_->setVisible(false);
}

void UdpConnectionWidget::setConnected(bool connected) {
    if (!connected) {
        setDisplayState(DisplayState::Disconnected);
        return;
    }
    setDisplayState(DisplayState::Bound);
}

void UdpConnectionWidget::applyDisplayState() {
    QString buttonText;
    QString statusText;
    QString statusStyle = QStringLiteral("color: red; font-weight: bold;");

    switch (displayState_) {
    case DisplayState::Disconnected:
        buttonText = tr("Bind");
        statusText = tr("Disconnected");
        break;
    case DisplayState::Connecting:
        buttonText = tr("Unbind");
        statusText = tr("Binding");
        statusStyle = QStringLiteral("color: orange; font-weight: bold;");
        break;
    case DisplayState::Bound:
        buttonText = tr("Unbind");
        statusText = tr("Bound");
        statusStyle = QStringLiteral("color: green; font-weight: bold;");
        break;
    case DisplayState::Disconnecting:
        buttonText = tr("Unbinding");
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
    if (remoteIpEdit_) remoteIpEdit_->setEnabled(inputsEnabled);
    if (remotePortEdit_) remotePortEdit_->setEnabled(inputsEnabled);
    connectBtn_->setEnabled(displayState_ != DisplayState::Disconnecting);

    updateProtocolUi();
}

void UdpConnectionWidget::updateProtocolUi() {
    if (displayState_ != DisplayState::Disconnected) {
        return;
    }
    hostLabel_->setText(tr("Local:"));
    connectBtn_->setText(tr("Bind"));
    if (remoteIpLabel_) remoteIpLabel_->setVisible(true);
    if (remoteIpEdit_) remoteIpEdit_->setVisible(true);
    if (remotePortLabel_) remotePortLabel_->setVisible(true);
    if (remotePortEdit_) remotePortEdit_->setVisible(true);
}

void UdpConnectionWidget::loadSettings() {
    NetworkConnectionWidget::loadSettings();
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    if (!remoteIpEdit_ || !remotePortEdit_) return;

    QSignalBlocker b1(remoteIpEdit_);
    QSignalBlocker b2(remotePortEdit_);

    remoteIpEdit_->setText(settingsService_->value(settingsGroup_ + QStringLiteral("/remoteIp")).toString());
    remotePortEdit_->setValue(settingsService_->contains(settingsGroup_ + QStringLiteral("/remotePort"))
        ? settingsService_->value(settingsGroup_ + QStringLiteral("/remotePort")).toInt() : 0);
}

void UdpConnectionWidget::saveSettings() {
    NetworkConnectionWidget::saveSettings();
    if (settingsGroup_.isEmpty() || !settingsService_) return;
    if (remoteIpEdit_) {
        settingsService_->setValue(settingsGroup_ + QStringLiteral("/remoteIp"), remoteIpEdit_->text());
    }
    if (remotePortEdit_) {
        settingsService_->setValue(settingsGroup_ + QStringLiteral("/remotePort"), remotePortEdit_->value());
    }
}

} // namespace ui::widgets