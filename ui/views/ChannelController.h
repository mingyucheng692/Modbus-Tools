// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QMetaObject>
#include <QThread>
#include <functional>
#include <memory>
#include <utility>
#include "common/ThreadGuard.h"

namespace io {
class ChannelOperationWorker;
}

namespace ui::views {

// Role: Manages worker thread lifecycle and channel communication dispatch for channel views.
// Thread-safety: Thread-compatible (bound to UI thread; dispatches to worker via QueuedConnection).
// Ownership: Owns worker QThread and ChannelOperationWorker instances.
class ChannelController : public QObject {
    Q_OBJECT

public:
    explicit ChannelController(QObject* parent = nullptr);
    ~ChannelController() noexcept override;

    io::ChannelOperationWorker* createWorker();
    void destroyWorker(QObject* receiverToDisconnect = nullptr);

    io::ChannelOperationWorker* worker() const { return worker_.get(); }
    bool hasWorker() const { return worker_ != nullptr; }

    void write(const QByteArray& data);
    void disconnect();

    void startAutoSend(int intervalMs, std::function<void()> sendAction);
    void stopAutoSend();
    void setAutoSendInterval(int intervalMs);
    [[nodiscard]] bool isAutoSendActive() const;

    // Releases a ThreadGuard-managed (QThread, Worker) pair cleanly:
    // disconnects business signals to receiverToDisconnect, triggers worker teardown
    // on the worker thread, waits up to 1000ms for the worker loop to exit, then
    // tears down the thread handle via ThreadGuard::releaseThread.
    template <typename Worker>
    static void releaseWorkerPair(std::shared_ptr<QThread>& thread,
                                  std::shared_ptr<Worker>& worker,
                                  QObject* receiverToDisconnect = nullptr) {
        if (worker && receiverToDisconnect) {
            QObject::disconnect(worker.get(), nullptr, receiverToDisconnect, nullptr);
        }
        auto localWorker = std::move(worker);
        auto localThread = std::move(thread);
        localWorker.reset();
        if (localThread && localThread->isRunning()
            && QThread::currentThread() != localThread.get()) {
            localThread->wait(1000);
        }
        localThread.reset();
    }

private:
    std::shared_ptr<QThread> workerThread_;
    std::shared_ptr<io::ChannelOperationWorker> worker_;
    QTimer* autoSendTimer_ = nullptr;
    std::function<void()> autoSendAction_;
};

} // namespace ui::views