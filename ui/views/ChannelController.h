/**
 * @file ChannelController.h
 * @brief Header file for ChannelController, a composite class managing worker thread
 *        lifecycle for channel views.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <functional>

namespace io {
class ChannelOperationWorker;
}

class QThread;

namespace ui::views {

/**
 * @class ChannelController
 * @brief Composite class that manages worker thread lifecycle.
 *
 * Owns the worker QThread and ChannelOperationWorker. Used as a member of
 * channel view subclasses (SerialDebuggerView, NetworkDebuggerView) to
 * eliminate duplicated worker lifecycle code.
 */
class ChannelController : public QObject {
    Q_OBJECT

public:
    explicit ChannelController(QObject* parent = nullptr);
    ~ChannelController() noexcept override;

    /// Creates a new worker thread and ChannelOperationWorker, starts the thread.
    /// Returns the worker for the caller to wire up signal connections.
    io::ChannelOperationWorker* createWorker();

    /// Destroys the worker and its thread.
    void destroyWorker();

    /// Returns the current worker, or nullptr if none exists.
    io::ChannelOperationWorker* worker() const { return worker_; }

    /// Returns true if a worker has been created and not destroyed.
    bool hasWorker() const { return worker_ != nullptr; }

    /// Invokes write() on the worker via queued connection.
    void write(const QByteArray& data);

    /// Invokes close() on the worker via queued connection.
    void disconnect();

    /// Starts periodic auto-sending using sendAction callback.
    void startAutoSend(int intervalMs, std::function<void()> sendAction);

    /// Stops active auto-sending.
    void stopAutoSend();

    /// Updates interval for active auto-send timer.
    void setAutoSendInterval(int intervalMs);

    /// Returns true if auto-send timer is currently running.
    [[nodiscard]] bool isAutoSendActive() const;

    /// Stops a (worker, thread) pair using deleteLater + quit pattern.
    /// Caller must nullify its member pointers before calling.
    /// @param onPreStop  invoked before shutdown logic.
    /// @param onCloseInThread  invoked on the worker thread before deleteLater
    ///                         (e.g., to call worker->close()).
    void stopWorkerPair(QThread* thread, QObject* worker,
                        const std::function<void()>& onPreStop = {},
                        const std::function<void(QObject*)>& onCloseInThread = {});

private:
    QThread* workerThread_ = nullptr;
    io::ChannelOperationWorker* worker_ = nullptr;
    QTimer* autoSendTimer_ = nullptr;
    std::function<void()> autoSendAction_;
};

} // namespace ui::views