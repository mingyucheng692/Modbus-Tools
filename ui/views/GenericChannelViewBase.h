/**
 * @file GenericChannelViewBase.h
 * @brief Header file for GenericChannelViewBase, the base class of generic channel views.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QWidget>
#include <QString>
#include <QByteArray>

namespace infra::config {
class ISettingsService;
}

namespace ui::widgets {
class GenericInputWidget;
}

namespace ui::views {

class ChannelController;

/**
 * @class GenericChannelViewBase
 * @brief Common base class for SerialDebuggerView and NetworkDebuggerView.
 *
 * Provides shared connection state tracking, settings access, and the
 * ChannelController pointer that subclasses initialize and use for worker
 * thread lifecycle management.
 */
class GenericChannelViewBase : public QWidget {
    Q_OBJECT

public:
    explicit GenericChannelViewBase(infra::config::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~GenericChannelViewBase() noexcept override;

protected slots:
    virtual void onDisconnectClicked();
    virtual void onSendRequested(const QByteArray& data);
    virtual void onAutoSendToggled(bool checked, int intervalMs);
    virtual void onAutoSendIntervalChanged(int intervalMs);

protected:
    virtual void retranslateUi() = 0;
    void changeEvent(QEvent* event) override;
    void bindInputWidget(widgets::GenericInputWidget* inputWidget);
    virtual void onAutoSendTimerTimeout();

    /// Subclasses initialize this in their constructor via setupUi().
    ChannelController* channelController_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;

    bool isConnected_ = false;

    /// Set by onDisconnectClicked(), cleared on the next user-initiated
    /// connect or on reaching Open. While true, passive disconnect alerts
    /// stay suppressed so a manual disconnect does not show an error popup.
    bool manualDisconnectRequested_ = false;

    infra::config::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::views