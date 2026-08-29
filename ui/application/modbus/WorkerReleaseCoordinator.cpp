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

    SPDLOG_INFO(
        "WorkerReleaseCoordinator: release requested worker_present={} channel_present={} "
        "worker_thread_running={} channel_thread_running={}",
        pending->worker != nullptr,
        pending->channel != nullptr,
        pending->workerThread && pending->workerThread->isRunning(),
        pending->channelThread && pending->channelThread->isRunning());

    // Empty handle: nothing to wait for. Emit completion via a queued
    // invocation so callers that chain on releaseCompleted() always observe a
    // consistent signal ordering (never a reentrant callback).
    if (pending->workerStopped && pending->channelThreadFinished
        && pending->workerThreadFinished
        && !pending->channel && !pending->client && !pending->worker) {
        SPDLOG_INFO("WorkerReleaseCoordinator: release completed immediately (empty handle)");
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
    // The channel thread is deliberately NOT quit here when a channel is
    // present: TcpChannel::close() and its linger fallback are event-loop
    // driven. Quitting the IO loop first leaves the socket stranded in
    // ConnectedState (no further events are ever delivered), and the
    // cross-thread destruction of that still-alive socket in finalize()
    // crashes in QCoreApplication::sendEvent. The quit is deferred to
    // beginChannelShutdown(), which first drives the channel to Closed.
    if (pending->channelThread && pending->channelThread->isRunning()
        && !pending->channel) {
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
    SPDLOG_INFO("WorkerReleaseCoordinator: worker reported stopped");
    pending->workerStopped = true;
    if (pending->workerThread && pending->workerThread->isRunning()) {
        pending->workerThread->quit();
    }
    beginChannelShutdown(pending);
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
        // The worker thread ended without a observed stopped() signal
        // (e.g. queued signal lost across thread teardown). The channel is
        // now unsupervised — start its shutdown sequence from here.
        if (!pending->workerStopped) {
            beginChannelShutdown(pending);
        }
    }
    tryComplete(pending);
}

void WorkerReleaseCoordinator::tryComplete(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        return;
    }
    // A dead worker thread is an acceptable substitute for the stopped()
    // signal: once the thread is gone no further channel access can originate
    // from the worker, so the channel shutdown/finalize may proceed.
    const bool workerDone = pending->workerStopped
        || !pending->workerThread || !pending->workerThread->isRunning();
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
        SPDLOG_ERROR(
            "WorkerReleaseCoordinator: shutdown timed out; finalizing without terminate()");
        emit releaseTimedOut(pending->timeoutMessage);
    }
    if (pending->workerThread && pending->workerThread->isRunning()) {
        pending->workerThread->quit();
    }
    // Do NOT finalize() directly: the channel may not be Closed yet, and
    // destroying a still-connected socket from the GUI thread crashes in
    // QCoreApplication::sendEvent. Drive the channel shutdown instead;
    // finalize() runs when the IO thread reports finished (with the force
    // timer as the ultimate backstop).
    beginChannelShutdown(pending);
}

void WorkerReleaseCoordinator::beginChannelShutdown(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending || pending->channelShutdownStarted) {
        return;
    }
    pending->channelShutdownStarted = true;

    if (!pending->channel) {
        if (pending->channelThread && pending->channelThread->isRunning()) {
            pending->channelThread->quit();
        }
        return;
    }

    if (!pending->channelThread || !pending->channelThread->isRunning()) {
        // IO loop already gone: close() takes its no-owner branch and sets
        // the terminal state synchronously (best effort — the socket itself
        // can no longer be closed at this point).
        pending->channel->close();
        tryComplete(pending);
        return;
    }

    if (pending->channel->state() == io::ChannelState::Closed) {
        // Already terminal (e.g. never opened): no close handshake needed,
        // and setState() would not re-notify the handler anyway.
        pending->channelThread->quit();
        tryComplete(pending);
        return;
    }

    // Observe the terminal Closed state and only then quit the IO thread.
    // The handler runs on the IO thread (inside ChannelBase::setState), so
    // it must not touch the coordinator directly — hop back to the GUI
    // thread via a queued invocation.
    pending->channelStateHandlerId = pending->channel->addStateHandler(
        [this, pending](io::ChannelState state) {
            if (state != io::ChannelState::Closed) {
                return;
            }
            QMetaObject::invokeMethod(this, [this, pending]() {
                detachChannelWatchers(pending);
                if (pending->channelThread
                    && pending->channelThread->isRunning()) {
                    pending->channelThread->quit();
                }
                tryComplete(pending);
            }, Qt::QueuedConnection);
        });
    // close() is invoked from the GUI thread; TcpChannel queues the real
    // work onto the (still running) IO thread. Its linger fallback
    // (kDefaultCloseLingerMs = 2s) guarantees Closed even for an
    // unresponsive peer that never ACKs the FIN.
    pending->channel->close();

    // Backstop: if Closed never arrives (e.g. the IO loop is wedged),
    // quit both threads anyway so finalize() is still reached. finalize()
    // leaks the channel instead of destroying it cross-thread when it is
    // not Closed — a leak is strictly better than the sendEvent assert.
    pending->channelForceTimer = new QTimer(this);
    pending->channelForceTimer->setSingleShot(true);
    QObject::connect(pending->channelForceTimer, &QTimer::timeout, this,
                     [this, pending]() {
                         detachChannelWatchers(pending);
                         if (pending->workerThread
                             && pending->workerThread->isRunning()) {
                             pending->workerThread->quit();
                         }
                         if (pending->channelThread
                             && pending->channelThread->isRunning()) {
                             pending->channelThread->quit();
                         }
                         tryComplete(pending);
                     });
    pending->channelForceTimer->start(kChannelShutdownForceMs);
}

void WorkerReleaseCoordinator::detachChannelWatchers(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        return;
    }
    if (pending->channelStateHandlerId && pending->channel) {
        pending->channel->removeStateHandler(pending->channelStateHandlerId);
        pending->channelStateHandlerId = 0;
    }
    if (pending->channelForceTimer) {
        pending->channelForceTimer->stop();
        pending->channelForceTimer->deleteLater();
        pending->channelForceTimer = nullptr;
    }
}

void WorkerReleaseCoordinator::finalize(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    const auto it = std::find(pending_.begin(), pending_.end(), pending);
    if (it == pending_.end()) {
        return;
    }
    pending_.erase(it);

    detachChannelWatchers(pending);

    if (pending->timeoutTimer) {
        pending->timeoutTimer->stop();
        pending->timeoutTimer->deleteLater();
        pending->timeoutTimer = nullptr;
    }

    if (pending->workerThread && pending->workerThread->isRunning()) {
        pending->workerThread->quit();
        if (!pending->workerThread->wait(1000)) {
            SPDLOG_WARN("WorkerReleaseCoordinator: worker thread did not finish in time");
        }
    }
    if (pending->channelThread && pending->channelThread->isRunning()) {
        pending->channelThread->quit();
        if (!pending->channelThread->wait(1000)) {
            SPDLOG_WARN("WorkerReleaseCoordinator: channel thread did not finish in time");
        }
    }

    pending->worker.reset();
    pending->client.reset();
    // Channel destruction safety: a channel that reached Closed has its
    // socket engine and notifier children already destroyed on the IO
    // thread, so destroying the (now inert) channel object from the GUI
    // thread is safe — ChannelBase's destruction guard accepts a stopped
    // owner thread.
    //
    // A channel still holding a LIVE socket (stranded Open/Closing/Error
    // after a wedged shutdown) must NOT be destroyed from this thread:
    // QObject teardown would deliver events to objects owned by the dead
    // IO thread and trip QCoreApplication::sendEvent's cross-thread assert
    // (observed crash with QNativeSocketEngine's notifier children). The
    // old moveToThread() call never worked — Qt requires the move to be
    // issued from the object's own thread — it only produced three
    // warnings while leaving the affinity untouched. Deliberately leak
    // instead: the OS reclaims the descriptor at process exit.
    if (pending->channel) {
        if (pending->channel->state() == io::ChannelState::Closed) {
            pending->channel.reset();
        } else {
            SPDLOG_CRITICAL(
                "WorkerReleaseCoordinator: channel not closed at finalize "
                "(state={}); leaking channel to avoid cross-thread "
                "destruction crash",
                static_cast<int>(pending->channel->state()));
            // Deliberate leak: replace the owning reference with a no-op
            // deleter so the channel object (and its stranded socket) is
            // never destroyed — process-exit reclamation by the OS is the
            // only safe teardown at this point.
            auto doomed = std::move(pending->channel);
            pending->channel = std::shared_ptr<io::IChannel>(
                doomed.get(), [](io::IChannel*) {});
            doomed.reset(); // releases the owning reference without deleting
        }
    }
    pending->channelThread.reset();
    pending->workerThread.reset();

    SPDLOG_INFO("WorkerReleaseCoordinator: release finalized");
    emit releaseCompleted();
}

} // namespace ui::application::modbus
