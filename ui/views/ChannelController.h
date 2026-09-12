/**
 * @file ChannelController.h
 * @brief Header file for ChannelController, a composite class managing worker thread
 *        lifecycle and reconnect timer for channel views.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <functional>
#include "../../infra/io/ReconnectPolicy.h"

namespace io {
class ChannelOperationWorker;
}

class QThread;

namespace ui::views {

/**
 * @class ChannelController
 * @brief Composite class that manages worker thread lifecycle and reconnect timer.
 *
 * Owns the worker QThread, ChannelOperationWorker, reconnect QTimer, and
 * ReconnectPolicy. Used as a member of channel view subclasses (SerialDebuggerView,
 * NetworkDebuggerView) to eliminate duplicated worker lifecycle code.
 */
class ChannelController : public QObject {
    Q_OBJECT

public:
    explicit ChannelController(QObject* parent = nullptr);
    ~ChannelController() noexcept override;

    /// Creates a new worker thread and ChannelOperationWorker, starts the thread.
    /// Returns the worker for the caller to wire up signal connections.
    io::ChannelOperationWorker* createWorker();

    /// Destroys the worker and its thread, stopping the reconnect timer first.
    void destroyWorker();

    /// Returns the current worker, or nullptr if none exists.
    io::ChannelOperationWorker* worker() const { return worker_; }

    /// Returns true if a worker has been created and not destroyed.
    bool hasWorker() const { return worker_ != nullptr; }

    /// Invokes write() on the worker via queued connection.
    void write(const QByteArray& data);

    /// Invokes close() on the worker via queued connection.
    void disconnect();

    // Reconnect timer management

    /// Starts the reconnect timer with the given delay, incrementing the retry count.
    void startReconnectTimer(int delayMs);

    /// Stops the reconnect timer.
    void stopReconnectTimer();

    /// Resets the reconnect policy (retry count to zero).
    void resetReconnect();

    /// Returns the reconnect policy for inspection.
    io::ReconnectPolicy& reconnectPolicy() { return reconnectPolicy_; }

    /// Returns the reconnect timer.
    QTimer* reconnectTimer() const { return reconnectTimer_; }

    /// Stops a (worker, thread) pair using deleteLater + quit pattern.
    /// Caller must nullify its member pointers before calling.
    /// @param onPreStop  invoked before shutdown logic (e.g., stop timers).
    /// @param onCloseInThread  invoked on the worker thread before deleteLater
    ///                         (e.g., to call worker->close()).
    void stopWorkerPair(QThread* thread, QObject* worker,
                        const std::function<void()>& onPreStop = {},
                        const std::function<void(QObject*)>& onCloseInThread = {});

signals:
    /// Emitted when the reconnect timer fires.
    void reconnectTimeout();

private:
    QThread* workerThread_ = nullptr;
    io::ChannelOperationWorker* worker_ = nullptr;
    QTimer* reconnectTimer_ = nullptr;
    io::ReconnectPolicy reconnectPolicy_;
};

} // namespace ui::views