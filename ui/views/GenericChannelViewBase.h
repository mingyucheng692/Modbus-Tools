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
#include <QTimer>
#include <QString>
#include <QByteArray>
#include <functional>
#include "../../infra/io/ReconnectPolicy.h"

namespace io {
class ChannelOperationWorker;
}

namespace ui::widgets {
class ByteMonitorWidget;
class GenericInputWidget;
class CollapsibleSection;
class BaseConnectionWidget;
}

class QThread;
class QEvent;

namespace core::common {
class ISettingsService;
}

namespace ui::views {

/**
 * @class GenericChannelViewBase
 * @brief Common base class for GenericSerialView and GenericTcpView.
 *
 * Manages the background worker thread lifecycle, common data transmission,
 * and reconnection timer behaviors.
 */
class GenericChannelViewBase : public QWidget {
    Q_OBJECT

public:
    explicit GenericChannelViewBase(core::common::ISettingsService* settingsService, QWidget* parent = nullptr);
    ~GenericChannelViewBase() noexcept override;

protected slots:
    virtual void onDisconnectClicked();
    virtual void onSendRequested(const QByteArray& data);
    virtual void onWorkerError(const QString& deviceHint, const QString& error);
    virtual void onWorkerMonitor(bool isTx, const QByteArray& data);
    virtual void onReconnectTimerTick() = 0;

protected:
    void startWorker();
    void stopWorker();

    // Stops a (worker, thread) pair using deleteLater + quit pattern.
    // Caller must nullify its member pointers before calling.
    // onPreStop: invoked before shutdown logic (e.g., stop timers).
    // onCloseInThread: invoked on the worker thread before deleteLater
    //                   (e.g., to call worker->close()).
    void stopWorkerPair(QThread* thread, QObject* worker,
                        const std::function<void()>& onPreStop = {},
                        const std::function<void(QObject*)>& onCloseInThread = {});

    void setupCommonWorkerConnections(io::ChannelOperationWorker* worker);
    void startReconnectTimer(widgets::BaseConnectionWidget* connectionWidget);
    void stopReconnectTimer();
    
    virtual void retranslateUi() = 0;
    void changeEvent(QEvent* event) override;

    // Common UI Components (Initialized by subclasses)
    widgets::ByteMonitorWidget* monitor_ = nullptr;
    widgets::GenericInputWidget* inputWidget_ = nullptr;
    widgets::CollapsibleSection* inputSection_ = nullptr;

    // Backend
    io::ChannelOperationWorker* worker_ = nullptr;
    QThread* workerThread_ = nullptr;
    bool isConnected_ = false;
    core::common::ISettingsService* settingsService_ = nullptr;

    io::ReconnectPolicy reconnectPolicy_;
    QTimer* reconnectTimer_ = nullptr;
};

} // namespace ui::views
