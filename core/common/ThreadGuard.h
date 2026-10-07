// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QMetaObject>
#include <QThread>
#include <memory>
#include <utility>

namespace core::common {

// Role: Safe teardown sequences for dedicated QThread, worker QObject, and IChannel pairs.
// Thread-safety: Thread-safe; static methods can be called during object teardown across threads.
// Ownership: Pure static utility namespace class; non-instantiable.
class ThreadGuard {
public:
    ThreadGuard() = delete; // static-only utility

    /// shared_ptr deleter / manual teardown for a dedicated QThread.
    static void releaseThread(QThread* thread)
    {
        if (!thread) {
            return;
        }

        if (thread->isRunning()) {
            thread->quit();
            if (QThread::currentThread() != thread && thread->wait(1000)) {
                delete thread;
                return;
            }
            // Timeout fallback: delayed self-delete once the loop finished.
            QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater,
                             Qt::UniqueConnection);
            return;
        }

        delete thread;
    }

    /// shared_ptr deleter for a worker QObject with no stop handshake.
    template <typename Worker>
    static void releaseWorker(Worker* worker)
    {
        releaseWorkerImpl(worker, [] {});
    }

    /// Teardown with a stop hook that runs on the worker thread before
    /// deleteLater (e.g. ModbusWorker::stop()).
    template <typename Worker, typename StopFn>
    static void releaseWorker(Worker* worker, StopFn stopInThread)
    {
        releaseWorkerImpl(worker, std::move(stopInThread));
    }

    // releaseChannel() contract
    ///      Teardown for a thread-affine channel (IChannel is NOT a QObject,
    ///      so it cannot serve as an invokeMethod context itself) that was
    ///      moved onto a dedicated IO thread. Because the channel shared_ptr
    ///      typically outlives its ModbusStack (ModbusClient keeps a channel
    ///      reference and the async-deleted worker keeps the client), the
    ///      deleter MUST capture the thread's shared_ptr — that capture is
    ///      what keeps the QThread object alive until after channel deletion,
    ///      so the channel's recorded owner-thread pointer never dangles.
    ///      - owner thread running: quit() + bounded wait (1 s), then delete
    ///        on the calling thread (quit/wait establishes happens-before and
    ///        the stopped owner satisfies ChannelBase's destruction guard).
    ///      - wait timeout: delayed delete once QThread::finished is
    ///        delivered (never an immediate cross-running-thread delete).
    ///      - already stopped / never started / already on the owner thread:
    ///        immediate delete.
    template <typename Channel>
    static void releaseChannel(Channel* channel, const std::shared_ptr<QThread>& ioThread)
    {
        if (!channel) {
            return;
        }

        QThread* owner = ioThread.get();
        if (owner && owner != QThread::currentThread() && owner->isRunning()) {
            owner->quit();
            if (!owner->wait(1000) && owner->isRunning()) {
                // Hung IO loop: delete once it finally finishes. The lambda
                // runs on the QThread object's own thread (the caller's)
                // after finished(); by then the loop is done and the
                // destruction guard accepts cross-thread deletion.
                QObject::connect(owner, &QThread::finished, owner,
                                 [channel]() { delete channel; },
                                 Qt::QueuedConnection);
                return;
            }
        }

        delete channel;
    }

private:
    template <typename Worker, typename StopFn>
    static void releaseWorkerImpl(Worker* worker, StopFn stopInThread)
    {
        if (!worker) {
            return;
        }

        QThread* objectThread = worker->thread();
        if (objectThread && objectThread->isRunning()) {
            QMetaObject::invokeMethod(worker, [worker, objectThread, stopInThread]() mutable {
                QObject::connect(worker, &QObject::destroyed, objectThread, &QThread::quit,
                                 Qt::UniqueConnection);
                stopInThread();
                worker->deleteLater();
            }, Qt::QueuedConnection);
            return;
        }

        delete worker;
    }
};

} // namespace core::common
