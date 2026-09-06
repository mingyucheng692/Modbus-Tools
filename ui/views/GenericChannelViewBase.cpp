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
#include "infra/config/ISettingsService.h"
#include <spdlog/spdlog.h>
#include <QEvent>

namespace ui::views {

GenericChannelViewBase::GenericChannelViewBase(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
}

GenericChannelViewBase::~GenericChannelViewBase() noexcept = default;

void GenericChannelViewBase::onDisconnectClicked() {
    if (!channelController_ || !channelController_->hasWorker()) {
        return;
    }

    // User intent flag (NEW-C): a manual disconnect must not be overridden
    // by the auto-reconnect loop. The loop only reacts to passive losses;
    // this flag is cleared on the next user-initiated connect. It also
    // suppresses the "connection lost" alert since the user caused it.
    manualDisconnectRequested_ = true;

    channelController_->stopReconnectTimer();
    channelController_->resetReconnect();
    channelController_->disconnect();
}

void GenericChannelViewBase::onSendRequested(const QByteArray& data) {
    if (!channelController_ || !channelController_->hasWorker()) {
        return;
    }
    if (!isConnected_) {
        SPDLOG_WARN("{}: send rejected (isConnected={}, dataSize={})",
                    metaObject()->className(), isConnected_, data.size());
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