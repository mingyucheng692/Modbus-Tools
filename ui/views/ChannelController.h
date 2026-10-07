// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <functional>

namespace io {
class ChannelOperationWorker;
}

class QThread;

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
    void destroyWorker();

    io::ChannelOperationWorker* worker() const { return worker_; }
    bool hasWorker() const { return worker_ != nullptr; }

    void write(const QByteArray& data);
    void disconnect();

    void startAutoSend(int intervalMs, std::function<void()> sendAction);
    void stopAutoSend();
    void setAutoSendInterval(int intervalMs);
    [[nodiscard]] bool isAutoSendActive() const;

    // Precondition: Caller must nullify its member pointers before calling.
    // Teardown sequence: executes onPreStop, invokes onCloseInThread on worker thread, then triggers deleteLater + quit.
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