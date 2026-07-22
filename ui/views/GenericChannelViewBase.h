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

namespace core::common {
class ISettingsService;
}

namespace ui::views {

class ChannelController;

/**
 * @class GenericChannelViewBase
 * @brief Common base class for GenericSerialView and GenericTcpView.
 *
 * Provides shared connection state tracking, settings access, and the
 * ChannelController pointer that subclasses initialize and use for worker
 * thread lifecycle and reconnect timer management.
 */
class GenericChannelViewBase : public QWidget {
    Q_OBJECT

public:
    explicit GenericChannelViewBase(core::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~GenericChannelViewBase() noexcept override;

protected slots:
    virtual void onDisconnectClicked();
    virtual void onSendRequested(const QByteArray& data);

protected:
    virtual void retranslateUi() = 0;
    void changeEvent(QEvent* event) override;

    /// Subclasses initialize this in their constructor via setupUi().
    ChannelController* channelController_ = nullptr;

    bool isConnected_ = false;
    core::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace ui::views