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
#include "../../../core/io/IChannel.h"
#include "../../../core/io/SerialConfig.h"

namespace ui::widgets {
class SerialConnectionWidget;
}

class QCheckBox;
class QGroupBox;

namespace ui::views::generic_serial {

class GenericSerialView : public GenericChannelViewBase {
    Q_OBJECT

public:
    explicit GenericSerialView(ui::common::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~GenericSerialView() noexcept override;

private slots:
    void onConnectClicked(const io::SerialConfig& config);
    void onWorkerStateChanged(io::ChannelState state);
    
    // Serial Control
    void onDtrChanged(bool checked);
    void onRtsChanged(bool checked);
    void onReconnectTimerTick() override;

protected:
    void startWorker();
    void retranslateUi() override;

    // UI Components
    widgets::SerialConnectionWidget* connectionWidget_ = nullptr;
    
    QCheckBox* dtrCheck_ = nullptr;
    QCheckBox* rtsCheck_ = nullptr;
    QGroupBox* controlGroup_ = nullptr;

    io::SerialConfig reconnectConfig_;

private:
    void setupUi();
};

} // namespace ui::views::generic_serial
