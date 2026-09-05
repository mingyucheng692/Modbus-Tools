/**
 * @file ConnectionManager.h
 * @brief Manages Modbus channel connection lifecycle and state.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ConnectionStateMachine.h"
#include "TimeoutHelper.h"
#include "RetryStrategy.h"
#include "infra/io/IChannel.h"
#include "../base/ModbusConfig.h"
#include <QString>

namespace modbus::session {

/**
 * @brief Encapsulates connection lifecycle: connect, disconnect, reconnect, and
 *        channel state waiting.
 *
 * @par Retry abstraction evaluation
 *      ensureConnected() uses an inline for-loop with RetryStrategy's static
 *      calculateBackoffMs() for delay computation. This is intentionally NOT
 *      migrated to RetryStrategy's instance API (shouldRetry/recordAttempt/
 *      nextWait) because: (1) the current loop has different retry-counting
 *      semantics (attempts = config->retries + 1 vs shouldRetry's <= maxRetries),
 *      (2) migrating would change runtime behavior and requires dedicated
 *      integration testing, (3) ReconnectPolicy (UI layer, fixed delay) and
 *      RetryStrategy (session layer, exponential backoff) serve different
 *      layers with different semantics and should NOT be unified into a single
 *      abstraction — forcing unification would be over-engineering.
 *
 * @note This class must not outlive the referenced objects (channel, stateMachine,
 *       timeoutController, config, mutex, cv). The owning ModbusClient is
 *       responsible for lifetime ordering.
 */
class ConnectionManager {
public:
    ConnectionManager(io::IChannel* channel,
                      ConnectionStateMachine* stateMachine,
                      std::atomic<bool>& aborted,
                      RetryStrategy* retryStrategy,
                      const base::ModbusConfig* config);

    /**
     * @brief Attempt to establish a connection with retry support.
     * @param allowReconnect Whether to retry on failure
     * @return true if connected successfully
     */
    bool ensureConnected(bool allowReconnect);

    /**
     * @brief Check if the underlying channel is open.
     */
    bool isConnected() const;

    /**
     * @brief Half-open detection: report a request timeout.
     *
     * Counts consecutive timeouts observed while the FSM reports Connected.
     * When the count reaches config->unresponsiveThreshold the session is
     * declared half-open (peer vanished without an RST — the socket stays
     * "connected" but is a black hole) and is evicted: FSM
     * Connected -> Failed ("unresponsive") followed by channel close(). The
     * next request then re-establishes the session through the lazy
     * reconnect path in RequestExecutor::sendRequestInternal().
     *
     * Timeouts observed outside Connected (connecting/reconnecting attempts
     * already own the FSM) do not count. Threshold <= 0 disables eviction.
     *
     * @thread Invoked by RequestExecutor on the worker thread. The counter is
     *         plain state guarded by that single-threaded driving contract.
     */
    void onRequestTimeout();

    /**
     * @brief Half-open detection: report a successful request.
     *
     * Any successful response proves the session is live and resets the
     * consecutive-timeout counter.
     */
    void onRequestSuccess();

    /**
     * @brief Get the last channel error message.
     * @note Thread-safe via internal mutex.
     */
    QString lastChannelError() const;
    bool hasChannelError() const;

    /**
     * @brief Clear the last channel error.
     */
    void clearError();

    /**
     * @brief Reset the failure latch so the next connection failure logs a warning.
     */
    void resetFailureLatch() noexcept { connectFailureLogged_ = false; }

    /**
     * @brief Set the last channel error (called from error callback).
     */
    void setError(const QString& error);

    /**
     * @brief Wait for the channel to reach a specific state.
     * @param expectedState The desired channel state
     * @param deadline Absolute deadline for the wait
     * @param errorOut If non-null, receives the error message on failure
     * @return true if the channel reached the expected state
     */
    bool waitForChannelState(io::ChannelState expectedState,
                             std::chrono::steady_clock::time_point deadline,
                             QString* errorOut);

private:
    io::IChannel* channel_;
    ConnectionStateMachine* stateMachine_;
    std::atomic<bool>& aborted_;
    RetryStrategy* retryStrategy_;
    const base::ModbusConfig* config_;
    QString lastChannelError_;
    /// Consecutive request timeouts observed while Connected (half-open
    /// detection). Worker-thread-only state — see onRequestTimeout().
    int consecutiveTimeoutCount_ = 0;

    /**
     * @brief Checked transition wrapper: logs (in addition to the FSM's own
     *        invalid-transition error) when a transition is rejected, so a
     *        rejected edge is never silently swallowed.
     * @return the tryTransition() result.
     */
    bool transitionChecked(ConnectionStateMachine::State to, const char* reason);

    /**
     * @brief Assert the FSM is in a clean entry state before a connect attempt.
     *
     * In normal operation (with abort guaranteeing terminal states), this is a
     * pass-through check. If a stale or intermediate state is detected, logs an
     * error and performs defensive self-healing to maintain production robustness.
     */
    void assertCleanEntryState();

    /// Failure latch: silences repetitive connection failure warnings under
    /// high-frequency polling when the peer is offline. Reset on connection success.
    bool connectFailureLogged_ = false;
};

} // namespace modbus::session
