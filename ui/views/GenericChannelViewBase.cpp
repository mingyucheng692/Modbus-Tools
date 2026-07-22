/**
 * @file GenericChannelViewBase.cpp
 * @brief Implementation of GenericChannelViewBase.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "GenericChannelViewBase.h"
#include "ChannelController.h"
#include "../common/ConnectionAlert.h"
#include <QEvent>

namespace ui::views {

GenericChannelViewBase::GenericChannelViewBase(core::common::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
}

GenericChannelViewBase::~GenericChannelViewBase() noexcept = default;

void GenericChannelViewBase::onDisconnectClicked() {
    if (!channelController_ || !channelController_->hasWorker()) {
        return;
    }

    channelController_->stopReconnectTimer();
    channelController_->resetReconnect();
    channelController_->disconnect();
}

void GenericChannelViewBase::onSendRequested(const QByteArray& data) {
    if (!channelController_ || !channelController_->hasWorker()) {
        return;
    }
    if (!isConnected_) {
        ui::common::connection_alert::showNotConnected(this);
        return;
    }
    channelController_->write(data);
}

void GenericChannelViewBase::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QWidget::changeEvent(event);
}

} // namespace ui::views