/**
 * @file UdpConnectionWidget.cpp
 * @brief Implementation of UdpConnectionWidget — lean subclass providing
 *        only protocol-specific display data, button wiring, and UDP-unique
 *        remote IP/Port fields.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UdpConnectionWidget.h"
#include "AppConstants.h"
#include "CollapsibleSection.h"
#include "../../core/common/ISettingsService.h"
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

UdpConnectionWidget::UdpConnectionWidget(core::common::ISettingsService* settingsService, QWidget* parent)
    : NetworkConnectionWidget(settingsService, parent) {
    // UDP needs extra remote-address controls inserted before the common
    // widgets (autoReconnect, connectBtn, etc.). We do this before calling
    // setupNetworkUi() so the layout is built first.
    setupCommonUi();

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

    // Now run the template-method setup
    setupProtocolUi();
    setupButtonConnection();
    loadSettings();
    updateProtocolUi();
    retranslateUi();
}

UdpConnectionWidget::~UdpConnectionWidget() = default;

// ---- Protocol-specific UI ----

void UdpConnectionWidget::setupProtocolUi() {
    hostLabel_->setText(tr("Local:"));
    connectBtn_->setText(tr("Bind"));
    remoteIpLabel_->setText(tr("Remote:"));
    remotePortLabel_->setText(tr("Remote Port:"));
    autoReconnectCheck_->setVisible(false);
    reconnectDelaySpin_->setVisible(false);
}

void UdpConnectionWidget::setupButtonConnection() {
    connect(connectBtn_, &QPushButton::clicked, this, [this]() {
        if (isActiveState(displayState_)) {
            emit unbindClicked();
        } else {
            saveSettings();
            emit bindClicked(ipEdit_->text(), portEdit_->value(),
                             remoteIpEdit_->text(), remotePortEdit_->value());
        }
    });

    connect(remoteIpEdit_, &QLineEdit::textChanged, this, &UdpConnectionWidget::saveSettings);
    connect(remotePortEdit_, qOverload<int>(&QSpinBox::valueChanged), this, &UdpConnectionWidget::saveSettings);
}

// ---- Display State Data ----

StateDisplayInfo UdpConnectionWidget::getStateDisplayInfo(DisplayState state) const {
    switch (state) {
    case DisplayState::Disconnected:
        return {tr("Bind"), tr("Disconnected"), QStringLiteral("color: red; font-weight: bold;")};
    case DisplayState::Connecting:
        return {tr("Unbind"), tr("Binding"), QStringLiteral("color: orange; font-weight: bold;")};
    case DisplayState::Bound:
        return {tr("Unbind"), tr("Bound"), QStringLiteral("color: green; font-weight: bold;")};
    case DisplayState::Disconnecting:
        return {tr("Unbinding"), tr("Disconnecting"), QStringLiteral("color: orange; font-weight: bold;")};
    default:
        return {};
    }
}

bool UdpConnectionWidget::isActiveState(DisplayState state) const {
    switch (state) {
    case DisplayState::Connecting:
    case DisplayState::Bound:
        return true;
    default:
        return false;
    }
}

NetworkConnectionWidget::DisplayState UdpConnectionWidget::connectedState() const {
    return DisplayState::Bound;
}

void UdpConnectionWidget::updateProtocolUi() {
    if (displayState_ != DisplayState::Disconnected) return;
    hostLabel_->setText(tr("Local:"));
    connectBtn_->setText(tr("Bind"));
    if (remoteIpLabel_) remoteIpLabel_->setVisible(true);
    if (remoteIpEdit_) remoteIpEdit_->setVisible(true);
    if (remotePortLabel_) remotePortLabel_->setVisible(true);
    if (remotePortEdit_) remotePortEdit_->setVisible(true);
}

// ---- Settings (UDP-specific remote fields) ----

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