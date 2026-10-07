// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QPointer>
#include <memory>
#include <vector>
#include <functional>

#include "infra/io/IChannel.h"
#include "modbus/session/SessionTypes.h"

class QThread;
class QTimer;

namespace modbus::dispatch { class ModbusWorker; }
namespace modbus::session { class ModbusClient; }

namespace ui::application::modbus {

// Role: Movable bundle holding ownership of live Modbus stack resources for async teardown.
// Thread-safety: Thread-compatible (movable value struct).
// Ownership: Transferred from Presenter to WorkerReleaseCoordinator.
struct StackHandle {
    std::shared_ptr<io::IChannel> channel;
    std::shared_ptr<::modbus::session::ModbusClient> client;
    std::shared_ptr<::modbus::dispatch::ModbusWorker> worker;
    std::shared_ptr<QThread> channelThread;
    std::shared_ptr<QThread> workerThread;

    /// True when the handle carries no live resource at all; used by the
    /// Coordinator to short-circuit the async dance.
    bool empty() const noexcept {
        return !worker && !client && !channel && !channelThread && !workerThread;
    }
};

// Role: Coordinates bounded, non-blocking asynchronous teardown of Modbus worker and channel threads.
// Thread-safety: Main-thread only (QObject derivative).
// Ownership: Owned by parent QObject (typically ModbusSessionPresenter).
class WorkerReleaseCoordinator : public QObject {
    Q_OBJECT

public:
    enum class ReleaseStage {
        Initial,
        StoppingWorker,
        ClosingChannel,
        JoiningThreads,
        TimedOut,
        Finalized
    };

    explicit WorkerReleaseCoordinator(QObject* parent = nullptr);
    ~WorkerReleaseCoordinator() noexcept override;

    /// Bounded default shutdown budget. If a worker stack has not fully
    /// stopped within this window the Coordinator finalizes the release
    /// without terminate() and emits releaseTimedOut.
    static constexpr int kDefaultTimeoutMs = 5000;

    /// Backstop budget for the channel shutdown sequence (close() -> Closed
    /// -> IO thread quit). TcpChannel's close linger lands Closed within
    /// kDefaultCloseLingerMs (2s); this timer fires only when the IO loop is
    /// fully wedged and forces the thread quit so finalize() still runs.
    static constexpr int kChannelShutdownForceMs = 3000;

    // Begins an asynchronous, bounded teardown of handle.
    // Empty handles complete synchronously via queued releaseCompleted emission.
    void requestRelease(StackHandle handle,
                        const QString& timeoutMessage,
                        int timeoutMs = 0);

    // Finalizes every still-pending release with timeoutMessage (used on shutdown).
    void shutdownAll(const QString& timeoutMessage);

    [[nodiscard]] bool hasPending() const noexcept;

    /// Test seam / observability: returns the current stage of the first active pending release,
    /// or ReleaseStage::Initial if no release is pending.
    [[nodiscard]] ReleaseStage currentStage() const noexcept;

signals:
    /// Emitted once per requestRelease() when the worker stack has fully
    /// stopped (or was empty to begin with).
    void releaseCompleted();

    /// Emitted when the bounded shutdown budget for a release elapses before
    /// all threads have joined. The release is still finalized safely.
    void releaseTimedOut(const QString& message);

private:
    struct PendingReleaseContext {
        std::shared_ptr<io::IChannel> channel;
        std::shared_ptr<::modbus::session::ModbusClient> client;
        std::shared_ptr<::modbus::dispatch::ModbusWorker> worker;
        std::shared_ptr<QThread> channelThread;
        std::shared_ptr<QThread> workerThread;

        ReleaseStage stage = ReleaseStage::Initial;
        bool workerStopped = false;         // ModbusWorker::stopped() signal received or workerThread exited
        bool channelThreadFinished = false; // IO channel thread finished() received (prerequisite for finalize())
        bool workerThreadFinished = false;  // Worker thread finished() signal received
        bool completionLogged = false;      // Prevents dual emission between onTimeout and tryComplete
        bool channelShutdownStarted = false;// Re-entrancy guard for beginChannelShutdown()
        // Note: The 5 flags above represent concurrent completion conditions (not mutually exclusive states).
        //       tryComplete() waits until all conditions are satisfied before calling finalize().

        QString timeoutMessage;
        QTimer* timeoutTimer = nullptr;
        /// Second-chance timer for the channel shutdown sequence (3-second fallback
        /// started in beginChannelShutdown): if Closed does not arrive in time, the
        /// IO thread is quit regardless so finalize() can still proceed.
        QTimer* channelForceTimer = nullptr;
        /// Channel state-handler subscription watching for the terminal
        /// Closed state that gates channelThread quit() (detached in detachChannelWatchers).
        io::IChannel::HandlerId channelStateHandlerId = 0;
    };

    void advanceStage(const std::shared_ptr<PendingReleaseContext>& pending, ReleaseStage next);
    void startRelease(std::shared_ptr<PendingReleaseContext> pending);
    void onWorkerStopped(const std::shared_ptr<PendingReleaseContext>& pending);
    void onThreadFinished(const std::shared_ptr<PendingReleaseContext>& pending,
                          bool isChannelThread);
    void tryComplete(const std::shared_ptr<PendingReleaseContext>& pending);
    void onTimeout(const std::shared_ptr<PendingReleaseContext>& pending);
    void finalize(const std::shared_ptr<PendingReleaseContext>& pending);

    /// Drives the channel's graceful shutdown and only then quits the
    ///        IO thread.
    ///
    /// The channel thread MUST stay alive until the channel actually reached
    /// Closed: TcpChannel::close() (and its linger fallback) are event-loop
    /// driven, so quitting the IO thread first strands the socket in
    /// ConnectedState forever. A channel destroyed in that state from the
    /// GUI thread trips QCoreApplication's cross-thread sendEvent assert
    /// while the socket engine's notifier children are torn down (observed
    /// crash with an unresponsive Modbus TCP peer). The sequence is:
    /// close() (queued to the IO thread) -> Closed observed -> quit().
    /// A bounded force timer aborts the wait so finalize() can still run.
    void beginChannelShutdown(const std::shared_ptr<PendingReleaseContext>& pending);

    /// Detaches the channel state-handler subscription and the force timer;
    /// called from finalize() and the Closed observer.
    void detachChannelWatchers(const std::shared_ptr<PendingReleaseContext>& pending);

    std::vector<std::shared_ptr<PendingReleaseContext>> pending_;
};

} // namespace ui::application::modbus
