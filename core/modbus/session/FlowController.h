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

class FlowController {
public:
    explicit FlowController(modbus::base::ModbusMode mode);

    void markWritePending();
    void markWriteDrained(std::chrono::steady_clock::time_point when);
    [[nodiscard]] bool isWriteDrained() const;
    [[nodiscard]] std::chrono::steady_clock::time_point drainedAt() const;

    void updateRtuSendWindow(std::ptrdiff_t frameBytes,
                             const modbus::base::ModbusConfig& config);
    [[nodiscard]] bool isRtuSendWindowOpen(
        std::chrono::steady_clock::time_point now) const;
    [[nodiscard]] std::chrono::steady_clock::time_point rtuSendWindowOpensAt() const;

    void reset();
    void setMode(modbus::base::ModbusMode mode);

private:
    modbus::base::ModbusMode mode_;
    bool writeDrained_ = true;
    std::chrono::steady_clock::time_point lastWriteDrainedAt_{};
    std::chrono::steady_clock::time_point nextRtuSendAllowedAt_{};
};

} // namespace modbus::session
