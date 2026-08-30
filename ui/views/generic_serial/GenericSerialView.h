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

namespace infra::config {
class ISettingsService;
}

namespace ui::views::generic_serial {

// ADR 0004 (design note): GenericSerialView / GenericTcpView deliberately hold
// their worker/channel directly and bypass the Presenter layer used by
// ModbusPage. Presenter is reserved for views with cross-cutting application
// state (session lifecycle, update flow, navigation); the generic_* views are
// self-contained channel pages with no shared presenter-worthy state, so a
// presenter here would be pure ceremony. Do not retro-fit a presenter without
// first extracting state worth abstracting.
class GenericSerialView : public GenericChannelViewBase {
    Q_OBJECT

public:
    explicit GenericSerialView(infra::config::ISettingsService* settingsService, QWidget *parent = nullptr);
    ~GenericSerialView() noexcept override;

private slots:
    void onConnectClicked(const io::SerialConfig& config);
    void onWorkerStateChanged(io::ChannelState state, quint64 generation);
    void onWorkerError(const QString& deviceHint, io::ChannelErrorCode code, const QString& error);
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

    /// Monotonic connection-attempt counter, bumped on every user connect /
    /// reconnect tick. Stale worker emissions from a torn-down channel carry
    /// an older generation and are dropped in onWorkerStateChanged() — the
    /// same mechanism GenericTcpView uses for rapid open/close sequences.
    quint64 connectionGeneration_ = 0;
};

} // namespace ui::views::generic_serial