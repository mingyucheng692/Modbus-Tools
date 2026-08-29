#pragma once

#include <QObject>
#include <QPointer>
#include <memory>
#include <vector>
#include <functional>

#include "../../../infra/io/IChannel.h"
#include "modbus/session/SessionTypes.h"

class QThread;
class QTimer;

namespace modbus::dispatch { class ModbusWorker; }
namespace modbus::session { class ModbusClient; }

namespace ui::application::modbus {

/**
 * @brief Movable bundle of the live Modbus stack resources handed off to the
 *        WorkerReleaseCoordinator for bounded asynchronous teardown.
 *
 * The Presenter fills a StackHandle with the owning shared_ptrs of the current
 * stack and moves it into requestRelease(); after the move the Presenter's own
 * fields are nullptr. The Coordinator then owns the lifetime until all threads
 * have joined (or the bounded timeout fires).
 */
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

/**
 * @brief Owns bounded, non-terminating teardown of Modbus worker stacks.
 *
 * @thread Lives on the GUI thread. All public methods must be called from the
 *         GUI thread. Worker threads are observed via queued QThread::finished
 *         connections; no blocking join is performed and QThread::terminate()
 *         is never used.
 *
 * @rationale Extracted from ModbusSessionPresenter to break up a God Class. The
 *            Coordinator is solely responsible for: tracking pending releases,
 *            wiring the worker/io-thread completion signals, enforcing a
 *            bounded timeout, and emitting completion/timeout signals. It does
 *            NOT touch session state (generation, linked flag, connection
 *            widget) — that stays in the Presenter, which reacts to the
 *            Coordinator's signals.
 */
class WorkerReleaseCoordinator : public QObject {
    Q_OBJECT

public:
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

    /**
     * @brief Begin an asynchronous, bounded teardown of @p handle.
     *
     * Empty handles complete synchronously: releaseCompleted is emitted
     * immediately (via a queued invocation on this object's thread) so callers
     * that rely on the signal still observe the transition.
     *
     * @param handle          Live stack resources to release (moved in).
     * @param timeoutMessage  Message surfaced through releaseTimedOut if the
     *                        bounded shutdown elapses.
     * @param timeoutMs       Bounded shutdown budget; 0 selects the default.
     */
    void requestRelease(StackHandle handle,
                        const QString& timeoutMessage,
                        int timeoutMs = 0);

    /**
     * @brief Finalize every still-pending release with the given timeout
     *        message.
     *
     * Used by the Presenter's destructor / explicit shutdown path so that no
     * worker stack outlives the owning Presenter.
     */
    void shutdownAll(const QString& timeoutMessage);

    /// @return true if at least one release is still outstanding.
    bool hasPending() const noexcept;

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
        bool workerStopped = false;
        bool channelThreadFinished = false;
        bool workerThreadFinished = false;
        bool completionLogged = false;
        /// Guards beginChannelShutdown() against re-entry (it is triggered
        /// from three paths: worker stopped, worker thread finished, timeout).
        bool channelShutdownStarted = false;
        QString timeoutMessage;
        QTimer* timeoutTimer = nullptr;
        /// Second-chance timer for the channel shutdown sequence: if Closed
        /// does not arrive in time (close linger should land within 2s), the
        /// IO thread is quit regardless so finalize() can still proceed.
        QTimer* channelForceTimer = nullptr;
        /// Channel state-handler subscription watching for the terminal
        /// Closed state that gates channelThread quit().
        io::IChannel::HandlerId channelStateHandlerId = 0;
    };

    void startRelease(std::shared_ptr<PendingReleaseContext> pending);
    void onWorkerStopped(const std::shared_ptr<PendingReleaseContext>& pending);
    void onThreadFinished(const std::shared_ptr<PendingReleaseContext>& pending,
                          bool isChannelThread);
    void tryComplete(const std::shared_ptr<PendingReleaseContext>& pending);
    void onTimeout(const std::shared_ptr<PendingReleaseContext>& pending);
    void finalize(const std::shared_ptr<PendingReleaseContext>& pending);

    /// @brief Drive the channel's graceful shutdown and only then quit the
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
