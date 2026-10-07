// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "ConnectionStateMachine.h"
#include "infra/io/IChannel.h"
#include "../base/ModbusConfig.h"
#include <QString>

namespace modbus::session {

// Role: Encapsulates connection lifecycle transitions and unresponsive session eviction.
// Thread-safety: Thread-compatible. Invoked by ModbusClient/RequestExecutor on owner thread.
// Ownership: Non-owning observer of channel, stateMachine, aborted flag, and config; owned by ModbusClient.
class ConnectionManager {
public:
    ConnectionManager(io::IChannel* channel,
                      ConnectionStateMachine* stateMachine,
                      std::atomic<bool>& aborted,
                      const base::ModbusConfig* config);

    bool ensureConnected();
    bool isConnected() const;

    // Half-open detection: reports request timeout and evicts session when consecutive timeouts exceed threshold.
    void onRequestTimeout();

    // Half-open detection: reports successful request, proving session is alive and resetting counter.
    void onRequestSuccess();

    QString lastChannelError() const;
    bool hasChannelError() const;
    void clearError();

    void resetFailureLatch() noexcept { connectFailureLogged_ = false; }

    void setError(const QString& error);

    // Waits for the channel to reach expectedState until deadline; writes error to errorOut on failure.
    bool waitForChannelState(io::ChannelState expectedState,
                             std::chrono::steady_clock::time_point deadline,
                             QString* errorOut);

private:
    io::IChannel* channel_;
    ConnectionStateMachine* stateMachine_;
    std::atomic<bool>& aborted_;
    const base::ModbusConfig* config_;
    QString lastChannelError_;
    /// Consecutive request timeouts observed while Connected (half-open
    /// detection). Worker-thread-only state — see onRequestTimeout().
    int consecutiveTimeoutCount_ = 0;

    // Checked transition wrapper: logs when a transition is rejected so edges are never silently swallowed.
    bool transitionChecked(ConnectionStateMachine::State to, const char* reason);

    // Asserts clean entry state before connect; logs error and performs defensive recovery on stale states.
    void assertCleanEntryState();

    /// Failure latch: silences repetitive connection failure warnings under
    /// high-frequency polling when the peer is offline. Reset on connection success.
    bool connectFailureLogged_ = false;
};

} // namespace modbus::session
