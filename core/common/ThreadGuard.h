/**
 * @file ThreadGuard.h
 * @brief Single project-wide implementation of the thread/worker teardown
 *        sequences (Plan v2 Task 3.5).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <QMetaObject>
#include <QThread>
#include <memory>
#include <utility>

namespace core::common {

/**
 * @brief Safe teardown sequences for dedicated QThread + worker-object pairs.
 *
 * This is the extraction (not a reinvention) of the two teardown sequences
 * that previously existed as twins in ModbusFactory.cpp and
 * FrameAnalyzerWidget.cpp. It exists exactly once in the project; do not
 * re-implement these patterns locally.
 *
 * @par releaseThread() contract
 *      quit() + bounded wait (1 s). When the wait succeeds the thread is
 *      deleted synchronously. When it times out, the ONLY permitted fallback
 *      is hooking QThread::finished -> deleteLater (delayed self-delete on
 *      the thread itself). An immediate delete after a wait timeout would be
 *      a use-after-free; the interface deliberately makes that impossible to
 *      write — there is no timeout-with-force-delete branch.
 *
 * @par releaseWorker() contract
 *      When the worker's thread is still running, deletion is queued onto
 *      that thread (deleteLater) with worker::destroyed chained to
 *      thread::quit, so the thread retires after the worker is gone; an
 *      optional stop hook (e.g. ModbusWorker::stop()) runs on the worker
 *      thread right before deleteLater. When the thread is already stopped,
 *      the worker is deleted directly on the calling thread.
 *
 * @par Member-destruction ordering requirement
 *      When thread and worker are held as shared_ptr members of the same
 *      object, declare the THREAD member first and the WORKER member second:
 *      members are destroyed in reverse order, so the worker's deleteLater is
 *      queued while the thread is still alive, and the subsequent
 *      releaseThread() quits + joins it (FIFO event delivery runs the queued
 *      deletion before the loop exits).
 */
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

    /// @par releaseChannel() contract
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
