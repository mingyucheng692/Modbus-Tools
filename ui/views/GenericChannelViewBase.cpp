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
#include "../widgets/GenericInputWidget.h"
#include "infra/config/ISettingsService.h"
#include <spdlog/spdlog.h>
#include <QEvent>

namespace ui::views {

GenericChannelViewBase::GenericChannelViewBase(infra::config::ISettingsService* settingsService, QWidget* parent)
    : QWidget(parent),
      settingsService_(settingsService) {
}

GenericChannelViewBase::~GenericChannelViewBase() noexcept = default;

void GenericChannelViewBase::bindInputWidget(widgets::GenericInputWidget* inputWidget) {
    inputWidget_ = inputWidget;
    if (!inputWidget_) {
        return;
    }
    connect(inputWidget_, &widgets::GenericInputWidget::sendRequested,
            this, &GenericChannelViewBase::onSendRequested);
    connect(inputWidget_, &widgets::GenericInputWidget::autoSendToggled,
            this, &GenericChannelViewBase::onAutoSendToggled);
    connect(inputWidget_, &widgets::GenericInputWidget::autoSendIntervalChanged,
            this, &GenericChannelViewBase::onAutoSendIntervalChanged);
}

void GenericChannelViewBase::onAutoSendToggled(bool checked, int intervalMs) {
    if (!channelController_) {
        return;
    }
    if (checked) {
        channelController_->startAutoSend(intervalMs, [this]() {
            onAutoSendTimerTimeout();
        });
    } else {
        channelController_->stopAutoSend();
    }
}

void GenericChannelViewBase::onAutoSendIntervalChanged(int intervalMs) {
    if (channelController_) {
        channelController_->setAutoSendInterval(intervalMs);
    }
}

void GenericChannelViewBase::onAutoSendTimerTimeout() {
    if (!inputWidget_) {
        return;
    }
    const QByteArray data = inputWidget_->getData();
    if (!data.isEmpty()) {
        onSendRequested(data);
    }
}

void GenericChannelViewBase::onDisconnectClicked() {
    if (!channelController_ || !channelController_->hasWorker()) {
        return;
    }

    if (channelController_->isAutoSendActive()) {
        channelController_->stopAutoSend();
        if (inputWidget_) {
            inputWidget_->setAutoSendEnabled(false);
        }
    }

    // Suppress the "connection lost" alert since the user caused it.
    manualDisconnectRequested_ = true;

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