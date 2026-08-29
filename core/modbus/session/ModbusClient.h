/**
 * @file ModbusClient.h
 * @brief Header file for ModbusClient.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "Config.h"
#include "SessionTypes.h"
#include "FrameExtractor.h"
#include "RetryStrategy.h"
#include "ConnectionStateMachine.h"
#include "RequestStateMachine.h"
#include "FlowController.h"
#include "TimeoutHelper.h"
#include "ConnectionManager.h"
#include "RequestExecutor.h"
#include "../transport/ITransport.h"
#include "infra/io/IChannel.h"
#include <atomic>
#include <deque>
#include <chrono>

class QThread;

namespace modbus::session {

/**
 * @brief Core Modbus client with full session management.
 *
 * @par Lazy reconnect contract (sendRequest)
 *      sendRequest() does NOT require a pre-established session: every
 *      request enters RequestExecutor::sendRequestInternal(), which calls
 *      ConnectionManager::ensureConnected(autoReconnect). With
 *      autoReconnect enabled a request against a dead session performs a
 *      bounded BLOCKING reconnect (attempts = retries+1 with reconnect
 *      backoff) before failing; with autoReconnect disabled it is a single
 *      attempt. Callers must therefore not gate submissions on
 *      isConnected() — that would defeat recovery under polling (the
 *      poller would spin on "Not connected" errors instead of rebuilding
 *      the session). The unresponsive eviction (half-open detection) and
 *      this lazy path together implement automatic recovery: consecutive
 *      timeouts evict the dead session, the next poll transparently
 *      reconnects.
 *
 * @thread thread-compatible, NOT thread-safe. The session-driving methods
 *         (connect, disconnect, sendRequest, sendRaw, setConfig) must be
 *         invoked from a single owning thread. In the application that owner
 *         is the ModbusWorker thread: ModbusWorker's QueuedConnection
 *         serialization is the ONLY concurrency boundary around this class.
 *         The happens-before chain is:
 *         UI thread submit -> QueuedConnection (Qt guarantees happens-before)
 *         -> worker thread handler -> requestFinished signal (queued)
 *         -> UI thread. Along that entire chain the client is only touched
 *         by the worker thread.
 *
 *         setConfig() has one additional legal caller: the thread that
 *         constructed the client (ModbusFactory applies the initial config
 *         before the worker takes ownership).
 *
 *         Documented cross-thread surface (no affinity requirement):
 *         - sessionHealth(): std::atomic load.
 *         - connectionState()/requestState(): benign enum snapshot reads
 *           used by the UI presenter; a torn read is impossible (plain
 *           enum), a stale value is acceptable for UI derivation.
 *         - isConnected()/lastChannelError(): mutex-guarded / plain reads.
 *         - abort(): the designated any-thread escape hatch (ModbusWorker::
 *           stop() and ~ModbusClient call it from foreign threads); it only
 *           flips the atomic aborted_ flag and notifies the wait CV.
 *
 *         Debug builds enforce the owner contract via assertSessionAffinity()
 *         on the session-driving entries. The guard is ARMED when ModbusWorker
 *         calls claimSessionOwnershipForCurrentThread() on the worker thread
 *         (first queued handler). Before arming — ModbusFactory's pre-live
 *         setConfig(), or core unit tests that drive the client directly
 *         (including from ad-hoc threads to exercise the Busy/abort defense
 *         paths) — affinity is deliberately not enforced.
 *
 * @note sendRequest() uses a separate requestMutex_ to prevent concurrent
 *       request execution. A stray concurrent submit is rejected at runtime
 *       with a structured Busy response (defense-in-depth for release
 *       builds where the Debug affinity assert is compiled out) — it is not
 *       an endorsement of multi-threaded driving.
 *
 * @par Architecture decision: ModbusClient / RequestExecutor split
 *      ModbusClient has 7 public methods that forward to RequestExecutor
 *      (sendRequest, sendRaw, abort) and ConnectionManager (connect,
 *      disconnect, isConnected, lastChannelError). The split is intentional:
 *      RequestExecutor (~830 LOC) handles request lifecycle, retry, response
 *      parsing; ConnectionManager handles connection lifecycle. Merging them
 *      into a single ~1000 LOC class would hurt readability without reducing
 *      complexity. The shared synchronization primitives (mutex_, cv_,
 *      aborted_) are passed by reference into RequestExecutor::Dependencies
 *      rather than extracted into a SessionSynchronizationContext wrapper —
 *      3 primitives do not justify a wrapper class.
 *
 * @par Synchronization primitive ownership
 *      mutex_, cv_ and aborted_ belong to ModbusClient; RequestExecutor and
 *      ConnectionManager are *authorized users* that receive them by
 *      reference via RequestExecutor::Dependencies. They are NOT a public
 *      thread-safety commitment: they exist to make the blocking wait loop
 *      and the abort handshake correct, and their locking rules are part of
 *      the internal contract between these three collaborators only.
 *
 * @par Destruction order
 *      The caller (typically ModbusWorker / WorkerReleaseCoordinator) MUST
 *      ensure the worker thread has fully joined (quit()+wait()) before
 *      ~ModbusClient() runs. If RequestExecutor::execute() is still running
 *      on the worker thread when the destructor fires, destroying
 *      requestExecutor_ and its collaborators is a use-after-free. The
 *      destructor asserts (release-mode, std::abort) that no request is
 *      in-flight (RequestStateMachine is Idle/Completed/Failed/Aborted).
 */
class ModbusClient {
public:
    using ConnectionState = ConnectionStateMachine::State;
    using RequestState = RequestStateMachine::State;

    ModbusClient(std::shared_ptr<io::IChannel> channel,
                 std::shared_ptr<transport::ITransport> transport);
    ~ModbusClient() noexcept;

    ModbusResponse sendRequest(const base::Pdu& request, int slaveId = -1);
    void sendRaw(const QByteArray& data);
    bool connect();
    void disconnect();
    /// Transport-level fact (delegates to ConnectionManager ->
    /// IChannel::isOpen()). NOT the session-level truth: a half-open socket
    /// reports connected here while ConnectionState already says Failed, and
    /// a session declared dead by the unresponsive eviction reports false
    /// here. For session truth use connectionState() == Connected.
    bool isConnected() const;
    QString lastChannelError() const;
    void abort();
    void setConfig(const base::ModbusConfig& config);
    ConnectionState connectionState() const;
    RequestState requestState() const;

    /// Session health: whether the Modbus device is actually responding,
    /// orthogonal to the transport-layer connection state.
    SessionHealth sessionHealth() const {
        return sessionHealth_.load(std::memory_order_acquire);
    }

    /// Debug-only contract arming: ModbusWorker invokes this on its own
    /// thread (from every queued handler that drives the client) to declare
    /// itself the owning thread. From then on, Debug builds assert that all
    /// session-driving methods run on that thread (see the @thread contract).
    /// No-op in Release builds.
#ifdef NDEBUG
    void claimSessionOwnershipForCurrentThread() {}
#else
    void claimSessionOwnershipForCurrentThread();
#endif

private:
    using PendingRequest = RequestExecutor::PendingRequest;

#ifdef NDEBUG
    // Release builds: the affinity contract is enforced by documentation and
    // ModbusWorker's QueuedConnection serialization; guards compile to nothing.
    void assertSessionAffinity() {}
#else
    // Debug-only owner-thread guard (see the @thread contract above). Armed
    // by claimSessionOwnershipForCurrentThread().
    void assertSessionAffinity();
    QThread* sessionOwnerThread_ = nullptr;
#endif

    bool ensureConnected(bool allowReconnect);
    bool waitForChannelState(io::ChannelState expectedState,
                             std::chrono::steady_clock::time_point deadline,
                             QString* errorOut);
    void clearRuntimeState(bool clearPendingQueue);

    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<transport::ITransport> transport_;
    base::ModbusConfig config_;

    // 同步机制：等待响应。所有权归 ModbusClient；RequestExecutor 与
    // ConnectionManager 通过 Dependencies 按引用获得授权使用权（见类注释
    // "Synchronization primitive ownership"）——它们不是公开的线程安全承诺。
    io::IChannel::HandlerId stateHandlerId_ = 0;
    
    std::atomic<bool> aborted_ {false};
    ConnectionStateMachine connectionStateMachine_;
    RetryStrategy retryStrategy_;
    ConnectionManager connectionManager_;
    RequestStateMachine requestStateMachine_;
    std::atomic<SessionHealth> sessionHealth_{SessionHealth::Unknown};

    FrameExtractor frameExtractor_;
    FlowController flowController_;

    RequestExecutor requestExecutor_;
};

} // namespace modbus::session
