/**
 * @file BaseConnectionWidget.cpp
 * @brief Implementation of BaseConnectionWidget.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "BaseConnectionWidget.h"
#include "CollapsibleSection.h"
#include "../common/ISettingsService.h"
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QSignalBlocker>

namespace ui::widgets {

BaseConnectionWidget::BaseConnectionWidget(ui::common::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
}

BaseConnectionWidget::~BaseConnectionWidget() = default;

void BaseConnectionWidget::setSettingsGroup(const QString& group) {
    settingsGroup_ = group;
    if (section_) {
        section_->setSettingsKey(settingsGroup_ + QStringLiteral("/ui/connectionSettingsCollapsed"));
    }
    loadSettings();
}

bool BaseConnectionWidget::autoReconnectEnabled() const noexcept {
    return autoReconnectCheck_ ? autoReconnectCheck_->isChecked() : false;
}

int BaseConnectionWidget::reconnectDelayMs() const noexcept {
    return reconnectDelaySpin_ ? reconnectDelaySpin_->value() : 3000;
}

void BaseConnectionWidget::setDisplayState(DisplayState state) {
    displayState_ = state;
    isConnected_ = (state == DisplayState::TransportConnected
                    || state == DisplayState::Connected
                    || state == DisplayState::Listening
                    || state == DisplayState::Bound);
    applyDisplayState();
}

void BaseConnectionWidget::createCommonWidgets(QWidget* parent) {
    autoReconnectCheck_ = new QCheckBox(parent);
    
    reconnectDelaySpin_ = new QSpinBox(parent);
    reconnectDelaySpin_->setRange(500, 30000);
    reconnectDelaySpin_->setValue(3000);
    reconnectDelaySpin_->setSuffix(QStringLiteral("ms"));
    reconnectDelaySpin_->setFixedWidth(76);

    connectBtn_ = new QPushButton(parent);
    
    statusLabel_ = new QLabel(parent);
    statusLabel_->setStyleSheet(QStringLiteral("color: red; font-weight: bold;"));
}

void BaseConnectionWidget::setupCommonConnections() {
    connect(autoReconnectCheck_, &QCheckBox::toggled, this, &BaseConnectionWidget::saveCommonSettings);
    connect(autoReconnectCheck_, &QCheckBox::toggled, this, [this]() {
        reconnectDelaySpin_->setVisible(autoReconnectCheck_->isChecked());
    });
    connect(reconnectDelaySpin_, qOverload<int>(&QSpinBox::valueChanged), this, &BaseConnectionWidget::saveCommonSettings);
}

void BaseConnectionWidget::loadCommonSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) {
        return;
    }
    
    QSignalBlocker b1(autoReconnectCheck_);
    QSignalBlocker b2(reconnectDelaySpin_);

    const QString autoReconnectKey = settingsGroup_ + QStringLiteral("/autoReconnect");
    const QString reconnectDelayKey = settingsGroup_ + QStringLiteral("/reconnectDelay");

    const bool autoReconnect = settingsService_->contains(autoReconnectKey)
        ? settingsService_->value(autoReconnectKey).toBool() : false;
    const int reconnectDelay = settingsService_->contains(reconnectDelayKey)
        ? settingsService_->value(reconnectDelayKey).toInt() : 3000;

    autoReconnectCheck_->setChecked(autoReconnect);
    reconnectDelaySpin_->setValue(reconnectDelay);
    reconnectDelaySpin_->setVisible(autoReconnect);
}

void BaseConnectionWidget::saveCommonSettings() {
    if (settingsGroup_.isEmpty() || !settingsService_) {
        return;
    }
    
    settingsService_->setValue(settingsGroup_ + QStringLiteral("/autoReconnect"), autoReconnectCheck_->isChecked());
    settingsService_->setValue(settingsGroup_ + QStringLiteral("/reconnectDelay"), reconnectDelaySpin_->value());
}

void BaseConnectionWidget::retranslateCommonUi() {
    if (section_) {
        section_->setTitle(tr("Connection Settings"));
    }
    if (autoReconnectCheck_) {
        autoReconnectCheck_->setText(tr("Auto Reconnect"));
    }
}

} // namespace ui::widgets
