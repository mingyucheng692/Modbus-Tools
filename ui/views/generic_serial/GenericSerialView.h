/**
 * @file GenericSerialView.h
 * @brief Header file for GenericSerialView.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../GenericChannelViewBase.h"
#include "../ChannelController.h"
#include "../../../infra/io/IChannel.h"
#include "../../../infra/io/SerialConfig.h"

namespace ui::widgets {
class SerialConnectionWidget;
class ByteMonitorWidget;
class GenericInputWidget;
class CollapsibleSection;
}

class QCheckBox;
class QGroupBox;

namespace core::common {
class ISettingsService;
}

namespace ui::views::generic_serial {

class GenericSerialView : public GenericChannelViewBase {
    Q_OBJECT

public:
    explicit GenericSerialView(core::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~GenericSerialView() noexcept override;

private slots:
    void onConnectClicked(const io::SerialConfig& config);
    void onWorkerStateChanged(io::ChannelState state);
    void onWorkerError(const QString& deviceHint, const QString& error);
    void onWorkerMonitor(bool isTx, const QByteArray& data);
    void onReconnectTimerTick();
    
    // Serial Control
    void onDtrChanged(bool checked);
    void onRtsChanged(bool checked);

protected:
    void retranslateUi() override;

private:
    void setupUi();
    void startWorker();

    // UI Components
    widgets::SerialConnectionWidget* connectionWidget_ = nullptr;
    widgets::ByteMonitorWidget* monitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    widgets::CollapsibleSection* inputSection_ = nullptr;
    
    QCheckBox* dtrCheck_ = nullptr;
    QCheckBox* rtsCheck_ = nullptr;
    QGroupBox* controlGroup_ = nullptr;

    // Channel controller (composite) – manages worker thread + reconnect timer
    ChannelController channelCtrl_;

    io::SerialConfig reconnectConfig_;
};

} // namespace ui::views::generic_serial