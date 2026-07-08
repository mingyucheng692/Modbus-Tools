/**
 * @file PendingSlaveTracker.h
 * @brief Tracks the in-flight request's expected slave ID for RTU/ASCII transports.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <cstdint>
#include <mutex>

namespace modbus::transport {

/**
 * @brief Thread-safe tracker for the pending request's expected slave ID.
 *
 * Encapsulates the pending-state mechanism shared by ModbusRtuTransport and
 * ModbusAsciiTransport: buildRequest() records which slave a response is
 * expected from; parseResponse() checks and clears it; resetPendingState()
 * clears it on abort/error.
 */
class PendingSlaveTracker {
public:
    /// Outcome of checking an incoming response against the pending request.
    enum class Outcome {
        NoPending,       ///< No request is in flight.
        SlaveMismatch,   ///< A request is in flight but slaveId does not match.
        Matched          ///< A request is in flight and slaveId matches; state cleared.
    };

    /// Records that a request is in-flight for @p slaveId.
    void setPending(uint8_t slaveId);

    /// Checks an incoming @p slaveId against the pending request.
    /// On match, clears the pending state and returns Outcome::Matched.
    /// On no-pending or mismatch, the state is unchanged.
    Outcome check(uint8_t slaveId);

    /// The expected slave ID of the currently pending request.
    /// Only meaningful when a request is pending.
    uint8_t expectedSlaveId() const;

    /// Clears any pending state.
    void reset();

private:
    mutable std::mutex mutex_;
    uint8_t expectedResponseSlaveId_ = 0;
    bool hasPendingRequest_ = false;
};

} // namespace modbus::transport
