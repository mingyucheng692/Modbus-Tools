/**
 * @file FlowController.h
 * @brief Flow control for Modbus write drain and RTU send window.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../base/ModbusConfig.h"
#include "../base/ModbusTypes.h"
#include <chrono>
#include <cstddef>

namespace modbus::session {

/**
 * @brief Flow control for Modbus write drain and RTU send window.
 *
 * @thread This class is not internally synchronized. It is owned by
 *         ModbusClient and accessed exclusively through RequestExecutor,
 *         which runs on the worker thread. All state mutations
 *         (markWritePending / markWriteDrained / updateRtuSendWindow /
 *         reset / setMode) MUST be performed while holding ModbusClient::mutex_
 *         so that visibility against concurrent channel-state reads is
 *         consistent. Read-only queries (isWriteDrained / drainedAt /
 *         isRtuSendWindowOpen / rtuSendWindowOpensAt) may be invoked without
 *         mutex_ when the caller has external serialization (RequestExecutor
 *         serializes all accesses via requestMutex_).
 */
class FlowController {
public:
    explicit FlowController(modbus::base::ModbusMode mode);

    // @pre Caller holds ModbusClient::mutex_.
    void markWritePending();
    // @pre Caller holds ModbusClient::mutex_.
    void markWriteDrained(std::chrono::steady_clock::time_point when);
    [[nodiscard]] bool isWriteDrained() const;
    [[nodiscard]] std::chrono::steady_clock::time_point drainedAt() const;

    // @pre Caller holds ModbusClient::mutex_.
    void updateRtuSendWindow(std::ptrdiff_t frameBytes,
                             const modbus::base::ModbusConfig& config);
    [[nodiscard]] bool isRtuSendWindowOpen(
        std::chrono::steady_clock::time_point now) const;
    [[nodiscard]] std::chrono::steady_clock::time_point rtuSendWindowOpensAt() const;

    // @pre Caller holds ModbusClient::mutex_.
    void reset();
    // @pre Caller holds ModbusClient::mutex_.
    void setMode(modbus::base::ModbusMode mode);

private:
    modbus::base::ModbusMode mode_;
    bool writeDrained_ = true;
    std::chrono::steady_clock::time_point lastWriteDrainedAt_{};
    std::chrono::steady_clock::time_point nextRtuSendAllowedAt_{};
};

} // namespace modbus::session
