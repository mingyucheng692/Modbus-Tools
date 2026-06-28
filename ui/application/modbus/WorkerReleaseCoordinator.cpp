#include "WorkerReleaseCoordinator.h"

#include "modbus/dispatch/ModbusWorker.h"

#include <QMetaObject>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace ui::application::modbus {

WorkerReleaseCoordinator::WorkerReleaseCoordinator(QObject* parent)
    : QObject(parent) {
}

WorkerReleaseCoordinator::~WorkerReleaseCoordinator() noexcept = default;

void WorkerReleaseCoordinator::requestRelease(StackHandle handle,
                                              const QString& timeoutMessage,
                                              int timeoutMs) {
    auto pending = std::make_shared<PendingReleaseContext>();
    pending->channel = std::move(handle.channel);
    pending->client = std::move(handle.client);
    pending->worker = std::move(handle.worker);
    pending->channelThread = std::move(handle.channelThread);
    pending->workerThread = std::move(handle.workerThread);
    pending->timeoutMessage = timeoutMessage;

    pending->workerStopped = !pending->worker;
    pending->channelThreadFinished =
        !pending->channelThread || !pending->channelThread->isRunning();
    pending->workerThreadFinished =
        !pending->workerThread || !pending->workerThread->isRunning();

    // Empty handle: nothing to wait for. Emit completion via a queued
    // invocation so callers that chain on releaseCompleted() always observe a
    // consistent signal ordering (never a reentrant callback).
    if (pending->workerStopped && pending->channelThreadFinished
        && pending->workerThreadFinished
        && !pending->channel && !pending->client && !pending->worker) {
        QMetaObject::invokeMethod(this, [this]() { emit releaseCompleted(); },
                                  Qt::QueuedConnection);
        return;
    }

    pending_.push_back(pending);

    // Bounded async shutdown: if completion does not arrive in time, finalize
    // safely without forcibly terminating threads.
    const int budget = timeoutMs > 0 ? timeoutMs : kDefaultTimeoutMs;
    pending->timeoutTimer = new QTimer(this);
    pending->timeoutTimer->setSingleShot(true);
    QObject::connect(pending->timeoutTimer, &QTimer::timeout, this,
                     [this, pending]() { onTimeout(pending); });
    pending->timeoutTimer->start(budget);

    if (pending->workerThread) {
        QObject::connect(pending->workerThread.get(), &QThread::finished, this,
                         [this, pending]() { onThreadFinished(pending, false); },
                         Qt::QueuedConnection);
    }
    if (pending->channelThread) {
        QObject::connect(pending->channelThread.get(), &QThread::finished, this,
                         [this, pending]() { onThreadFinished(pending, true); },
                         Qt::QueuedConnection);
    }

    if (pending->workerThread && pending->workerThread->isRunning()) {
        pending->workerThread->requestInterruption();
    }
    if (pending->channelThread && pending->channelThread->isRunning()) {
        pending->channelThread->requestInterruption();
    }

    if (pending->worker) {
        QObject::connect(pending->worker.get(),
                         &::modbus::dispatch::ModbusWorker::stopped, this,
                         [this, pending]() { onWorkerStopped(pending); },
                         Qt::QueuedConnection);
        pending->worker->stop();
    }
    if (pending->workerThread && pending->workerThread->isRunning()) {
        pending->workerThread->quit();
    }
    if (pending->channelThread && pending->channelThread->isRunning()) {
        pending->channelThread->quit();
    }
    tryComplete(pending);
}

void WorkerReleaseCoordinator::shutdownAll(const QString& timeoutMessage) {
    if (pending_.empty()) {
        return;
    }
    // Stamp the timeout message on every still-pending release and let their
    // bounded timers finalize them. We do not force-finalize here because the
    // bounded path already guarantees progress.
    for (auto& pending : pending_) {
        if (pending) {
            pending->timeoutMessage = timeoutMessage;
        }
    }
}

bool WorkerReleaseCoordinator::hasPending() const noexcept {
    return !pending_.empty();
}

void WorkerReleaseCoordinator::onWorkerStopped(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        return;
    }
    pending->workerStopped = true;
    if (pending->workerThread && pending->workerThread->isRunning()) {
        pending->workerThread->quit();
    }
    if (pending->channelThread && pending->channelThread->isRunning()) {
        pending->channelThread->quit();
    }
    tryComplete(pending);
}

void WorkerReleaseCoordinator::onThreadFinished(
    const std::shared_ptr<PendingReleaseContext>& pending, bool isChannelThread) {
    if (!pending) {
        return;
    }
    if (isChannelThread) {
        pending->channelThreadFinished = true;
    } else {
        pending->workerThreadFinished = true;
    }
    tryComplete(pending);
}

void WorkerReleaseCoordinator::tryComplete(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        return;
    }
    const bool workerDone = pending->workerStopped;
    const bool channelDone =
        pending->channelThreadFinished || !pending->channelThread
        || !pending->channelThread->isRunning();
    const bool workerThreadDone =
        pending->workerThreadFinished || !pending->workerThread
        || !pending->workerThread->isRunning();
    if (!workerDone || !channelDone || !workerThreadDone) {
        return;
    }
    finalize(pending);
}

void WorkerReleaseCoordinator::onTimeout(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        return;
    }
    const auto it = std::find(pending_.begin(), pending_.end(), pending);
    if (it == pending_.end()) {
        return;
    }
    if (!pending->completionLogged) {
        pending->completionLogged = true;
        spdlog::error(
            "WorkerReleaseCoordinator: shutdown timed out; finalizing without terminate()");
        emit releaseTimedOut(pending->timeoutMessage);
    }
    if (pending->workerThread && pending->workerThread->isRunning()) {
        pending->workerThread->quit();
    }
    if (pending->channelThread && pending->channelThread->isRunning()) {
        pending->channelThread->quit();
    }
    finalize(pending);
}

void WorkerReleaseCoordinator::finalize(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    const auto it = std::find(pending_.begin(), pending_.end(), pending);
    if (it == pending_.end()) {
        return;
    }
    pending_.erase(it);

    if (pending->timeoutTimer) {
        pending->timeoutTimer->stop();
        pending->timeoutTimer->deleteLater();
        pending->timeoutTimer = nullptr;
    }

    if (pending->workerThread && pending->workerThread->isRunning()) {
        pending->workerThread->quit();
    }
    if (pending->channelThread && pending->channelThread->isRunning()) {
        pending->channelThread->quit();
    }

    pending->worker.reset();
    pending->client.reset();
    pending->channel.reset();
    pending->channelThread.reset();
    pending->workerThread.reset();

    emit releaseCompleted();
}

} // namespace ui::application::modbus
