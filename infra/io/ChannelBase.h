// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "IChannel.h"
#include <atomic>
#include <mutex>
#include <vector>
#include <QThread>
#include <QMetaObject>

namespace io {

// Role: Common base class for IO channels providing thread affinity, metrics, and event callbacks.
// Thread-safety: Thread-compatible; state and callbacks protected by internal synchronization or thread affinity.
// Ownership: Abstract base class; owned by communication stack via std::shared_ptr<IChannel>.
class ChannelBase : public IChannel {
public:
    ChannelState state() const override;
    void moveToThread(QThread* thread) override;
    bool isOpen() const override;
    void setTimeouts(const Timeouts& timeouts) override;
    Timeouts timeouts() const override;
    void setReadHandler(std::function<void(QByteArrayView)> handler) override;
    void setErrorHandler(std::function<void(const ChannelError&)> handler) override;
    void setWriteDrainedHandler(std::function<void()> handler) override;
    HandlerId addStateHandler(std::function<void(ChannelState)> handler) override;
    void removeStateHandler(HandlerId handlerId) override;
    void setMonitor(std::function<void(bool isTx, const QByteArray&)> monitor) override;
    ChannelStats stats() const override;

protected:
    void setState(ChannelState state);
    void addTx(qsizetype bytes);
    void addRx(qsizetype bytes);
    void emitRead(QByteArrayView data);
    void emitError(ChannelErrorCode code, const QString& error);
    void emitWriteDrained();
    void emitMonitor(bool isTx, const QByteArray& data);

    // Marshals func onto the owner QObject's thread via Qt::QueuedConnection if called from another thread.
    template <typename QObj, typename Func>
    static bool marshalToOwner(QObj& owner, Func&& func) {
        if (QThread::currentThread() != owner.thread()) {
            QMetaObject::invokeMethod(&owner, std::forward<Func>(func),
                                      Qt::QueuedConnection);
            return true;
        }
        return false;
    }

    /// Asserts (debug-only) that the current thread is the owner's thread.
    template <typename QObj>
    static void assertOwnerThread(const QObj& owner, const char* context) {
        Q_ASSERT_X(QThread::currentThread() == owner.thread(),
                   context, "must run on the owner thread");
    }

    /// Release-mode UAF guard: aborts if the current thread is not the
    /// device owner thread set by the last moveToThread() call.
    void assertOwnerThreadForDestruction(const char* className);

private:
    std::atomic<ChannelState> state_{ChannelState::Closed};
    // timeouts_ is accessed from both IO thread (read in write timeout checks)
    // and GUI thread (setTimeouts during config). Protected by timeoutsMutex_
    // since setTimeouts is rare (config-time only), not a hot path.
    mutable std::mutex timeoutsMutex_;
    Timeouts timeouts_{};
    // stats_ counters are on the IO-thread hot path (addTx/addRx per frame).
    // Use relaxed atomics — no ordering guarantee needed for byte counters.
    std::atomic<quint64> bytesTx_{0};
    std::atomic<quint64> bytesRx_{0};
    std::function<void(QByteArrayView)> readHandler_;
    std::function<void(const ChannelError&)> errorHandler_;
    std::function<void()> writeDrainedHandler_;
    std::function<void(bool, const QByteArray&)> monitor_;
    std::mutex handlerMutex_;  // protects readHandler_, errorHandler_, writeDrainedHandler_, monitor_
    std::mutex stateHandlersMutex_;
    HandlerId nextStateHandlerId_ = 1;
    std::vector<std::pair<HandlerId, std::function<void(ChannelState)>>> stateHandlers_;
    // Owner thread starts as the constructing thread; moveToThread() updates
    // it. Starting at the creator (not nullptr) keeps the destruction guard
    // from firing for channels that are never moved off their birth thread
    // (e.g. unit tests constructing and destroying in main()).
    QThread* deviceThread_ = QThread::currentThread();
};

}
