// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

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

#include <QPointer>
#include <mutex>

namespace {

void leakChannelSafely(std::shared_ptr<io::IChannel> channel) {
    if (channel) {
        new std::shared_ptr<io::IChannel>(std::move(channel));
    }
}

} // namespace

struct WorkerReleaseCoordinator::AliveToken {
    std::mutex mutex;
    bool alive{true};
};

WorkerReleaseCoordinator::~WorkerReleaseCoordinator() noexcept {
    if (aliveToken_) {
        std::lock_guard<std::mutex> lock(aliveToken_->mutex);
        aliveToken_->alive = false;
    }

    const auto stopThread = [this](const std::shared_ptr<QThread>& thread) {
        if (!thread) {
            return;
        }
        thread->disconnect(this);
        if (thread->isRunning() && thread.get() != QThread::currentThread()) {
            thread->quit();
            thread->wait(1000);
        }
    };

    auto contexts = std::move(pending_);
    for (const auto& pending : contexts) {
        if (!pending) {
            continue;
        }
        if (pending->timeoutTimer) {
            pending->timeoutTimer->stop();
            pending->timeoutTimer->disconnect(this);
        }
        detachChannelWatchers(pending);
        if (pending->worker) {
            pending->worker->disconnect(this);
        }
        if (pending->channel) {
            pending->channel->close();
        }
        stopThread(pending->thread);
        releaseChannelSafely(pending);
    }
}

void WorkerReleaseCoordinator::quitThreadIfSafe(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending || !pending->thread || !pending->thread->isRunning()) {
        return;
    }
    if (pending->worker && !pending->workerStopped) {
        return;
    }
    if (pending->channel && pending->channel->state() != io::ChannelState::Closed) {
        return;
    }
    pending->thread->quit();
}

void WorkerReleaseCoordinator::releaseChannelSafely(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        return;
    }
    const bool threadRunning = pending->thread
        && pending->thread->isRunning()
        && pending->thread.get() != QThread::currentThread();
    if (threadRunning) {
        QThread* deferThread = pending->thread.get();
        QObject::connect(deferThread, &QThread::finished, deferThread,
                         [ch = std::move(pending->channel),
                          wk = std::move(pending->worker),
                          th = pending->thread]() mutable {
                             wk.reset();
                             ch.reset();
                         },
                         Qt::QueuedConnection);
        return;
    }
    pending->worker.reset();
    if (pending->channel) {
        if (pending->channel->state() == io::ChannelState::Closed) {
            pending->channel.reset();
        } else {
            leakChannelSafely(std::move(pending->channel));
        }
    }
}

void WorkerReleaseCoordinator::requestRelease(StackHandle handle,
                                              const QString& timeoutMessage,
                                              int timeoutMs) {
    auto pending = std::make_shared<PendingReleaseContext>();
    pending->channel = std::move(handle.channel);
    pending->client = std::move(handle.client);
    pending->worker = std::move(handle.worker);
    pending->thread = std::move(handle.thread);
    pending->timeoutMessage = timeoutMessage;

    pending->workerStopped = !pending->worker;
    pending->threadFinished =
        !pending->thread || !pending->thread->isRunning();

    SPDLOG_DEBUG(
        "WorkerReleaseCoordinator: release requested worker_present={} channel_present={} "
        "thread_running={}",
        pending->worker != nullptr,
        pending->channel != nullptr,
        pending->thread && pending->thread->isRunning());

    // Empty handle: nothing to wait for. Emit completion via a queued
    // invocation so callers that chain on releaseCompleted() always observe a
    // consistent signal ordering (never a reentrant callback).
    if (pending->workerStopped && pending->threadFinished
        && !pending->channel && !pending->client && !pending->worker) {
        SPDLOG_DEBUG("WorkerReleaseCoordinator: release completed immediately (empty handle)");
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

    if (pending->thread) {
        QObject::connect(pending->thread.get(), &QThread::finished, this,
                         [this, pending]() { onThreadFinished(pending); },
                         Qt::QueuedConnection);
        if (pending->thread->isRunning()) {
            pending->thread->requestInterruption();
        }
    }

    if (pending->worker) {
        QObject::connect(pending->worker.get(),
                         &::modbus::dispatch::ModbusWorker::stopped, this,
                         [this, pending]() { onWorkerStopped(pending); },
                         Qt::QueuedConnection);
        pending->worker->stop();
    }
    quitThreadIfSafe(pending);
    if (pending->thread && pending->thread->isRunning()
        && !pending->channel && !pending->worker) {
        pending->thread->quit();
    }
    advanceStage(pending, ReleaseStage::StoppingWorker);
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

WorkerReleaseCoordinator::ReleaseStage WorkerReleaseCoordinator::currentStage() const noexcept {
    if (!pending_.empty() && pending_.front()) {
        return pending_.front()->stage;
    }
    return ReleaseStage::Initial;
}

void WorkerReleaseCoordinator::advanceStage(
    const std::shared_ptr<PendingReleaseContext>& pending, ReleaseStage next) {
    if (!pending) {
        return;
    }
    // Terminal idempotency: once finalized, ignore any further stage advances.
    if (pending->stage == ReleaseStage::Finalized) {
        return;
    }

    // Monotonic progression:
    // TimedOut and Finalized can always be transitioned to from active stages.
    if (next != ReleaseStage::TimedOut && next != ReleaseStage::Finalized
        && static_cast<int>(next) <= static_cast<int>(pending->stage)) {
        return;
    }

    const auto prevStage = pending->stage;
    pending->stage = next;
    SPDLOG_DEBUG("WorkerReleaseCoordinator: stage {} -> {}",
                 static_cast<int>(prevStage), static_cast<int>(next));

    switch (next) {
    case ReleaseStage::Initial:
    case ReleaseStage::StoppingWorker:
    case ReleaseStage::JoiningThreads:
        break;
    case ReleaseStage::ClosingChannel:
        beginChannelShutdown(pending);
        break;
    case ReleaseStage::Finalized:
        finalize(pending);
        break;
    case ReleaseStage::TimedOut:
        if (!pending->completionLogged) {
            pending->completionLogged = true;
            SPDLOG_ERROR(
                "WorkerReleaseCoordinator: shutdown timed out; finalizing without terminate()");
            emit releaseTimedOut(pending->timeoutMessage);
        }
        quitThreadIfSafe(pending);
        beginChannelShutdown(pending);
        break;
    }
}

void WorkerReleaseCoordinator::onWorkerStopped(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        return;
    }
    SPDLOG_DEBUG("WorkerReleaseCoordinator: worker reported stopped");
    pending->workerStopped = true;
    quitThreadIfSafe(pending);
    advanceStage(pending, ReleaseStage::ClosingChannel);
    tryComplete(pending);
}

void WorkerReleaseCoordinator::onThreadFinished(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        return;
    }
    pending->threadFinished = true;
    tryComplete(pending);
}

void WorkerReleaseCoordinator::tryComplete(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending) {
        return;
    }
    // A dead thread is an acceptable substitute for the stopped()
    // signal: once the thread is gone no further channel access can originate
    // from the worker, so the channel shutdown/finalize may proceed.
    const bool workerDone = !pending->worker || pending->workerStopped
        || !pending->thread || !pending->thread->isRunning();
    const bool threadDone =
        pending->threadFinished || !pending->thread
        || !pending->thread->isRunning();
    if (!workerDone || !threadDone) {
        if (pending->stage == ReleaseStage::StoppingWorker && workerDone) {
            advanceStage(pending,
                         pending->channel ? ReleaseStage::ClosingChannel
                                          : ReleaseStage::JoiningThreads);
        }
        return;
    }
    advanceStage(pending, ReleaseStage::Finalized);
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
    advanceStage(pending, ReleaseStage::TimedOut);
    advanceStage(pending, ReleaseStage::Finalized);
}

void WorkerReleaseCoordinator::beginChannelShutdown(
    const std::shared_ptr<PendingReleaseContext>& pending) {
    if (!pending || pending->channelShutdownStarted) {
        return;
    }
    pending->channelShutdownStarted = true;

    if (!pending->channel) {
        if (pending->thread && pending->thread->isRunning()) {
            pending->thread->quit();
        }
        return;
    }

    if (!pending->thread || !pending->thread->isRunning()) {
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
        pending->thread->quit();
        tryComplete(pending);
        return;
    }

    // Observe the terminal Closed state and only then quit the IO thread.
    // The handler runs on the IO thread (inside ChannelBase::setState), so
    // it must not touch the coordinator directly — hop back to the GUI
    // thread via a queued invocation.
    if (!aliveToken_) {
        aliveToken_ = std::make_shared<AliveToken>();
    }
    const auto aliveToken = aliveToken_;
    const std::weak_ptr<PendingReleaseContext> weakPending = pending;
    const QPointer<WorkerReleaseCoordinator> self(this);
    pending->channelStateHandlerId = pending->channel->addStateHandler(
        [self, aliveToken, weakPending](io::ChannelState state) {
            if (state != io::ChannelState::Closed) {
                return;
            }
            std::lock_guard<std::mutex> lock(aliveToken->mutex);
            if (!aliveToken->alive || !self) {
                return;
            }
            QMetaObject::invokeMethod(self.data(), [self, aliveToken, weakPending]() {
                {
                    std::lock_guard<std::mutex> innerLock(aliveToken->mutex);
                    if (!aliveToken->alive || !self) {
                        return;
                    }
                }
                const auto activePending = weakPending.lock();
                if (!activePending) {
                    return;
                }
                self->detachChannelWatchers(activePending);
                if (activePending->thread
                    && activePending->thread->isRunning()) {
                    activePending->thread->quit();
                }
                self->tryComplete(activePending);
            }, Qt::QueuedConnection);
        });
    // close() is invoked from the GUI thread; TcpChannel queues the real
    // work onto the (still running) IO thread. Its linger fallback
    // (kDefaultCloseLingerMs = 2s) guarantees Closed even for an
    // unresponsive peer that never ACKs the FIN.
    pending->channel->close();

    // Backstop: if Closed never arrives (e.g. the IO loop is wedged),
    // quit thread anyway and force finalize so pending_ does not stall forever.
    pending->channelForceTimer = new QTimer(this);
    pending->channelForceTimer->setSingleShot(true);
    QObject::connect(pending->channelForceTimer, &QTimer::timeout, this,
                     [this, pending]() {
                         advanceStage(pending, ReleaseStage::TimedOut);
                         detachChannelWatchers(pending);
                         if (pending->thread
                             && pending->thread->isRunning()) {
                             pending->thread->quit();
                         }
                         advanceStage(pending, ReleaseStage::Finalized);
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

    if (pending->thread && pending->thread->isRunning()) {
        pending->thread->quit();
        if (!pending->thread->wait(1000)) {
            SPDLOG_WARN("WorkerReleaseCoordinator: thread did not finish in time");
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
    // after a wedged shutdown) must NOT be destroyed from this thread.
    // Deliberately leak instead: the OS reclaims the descriptor at process exit.
    if (pending->channel) {
        if (pending->channel->state() == io::ChannelState::Closed) {
            pending->channel.reset();
        } else {
            SPDLOG_CRITICAL(
                "WorkerReleaseCoordinator: channel not closed at finalize "
                "(state={}); leaking channel to avoid cross-thread "
                "destruction crash",
                static_cast<int>(pending->channel->state()));
            leakChannelSafely(std::move(pending->channel));
        }
    }
    pending->thread.reset();
    pending->stage = ReleaseStage::Finalized;

    SPDLOG_DEBUG("WorkerReleaseCoordinator: release finalized");
    emit releaseCompleted();
}

} // namespace ui::application::modbus
