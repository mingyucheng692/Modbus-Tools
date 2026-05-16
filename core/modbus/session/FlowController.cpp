/**
 * @file FlowController.cpp
 * @brief Implementation of FlowController.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FlowController.h"
#include "FrameExtractor.h"

namespace modbus::session {

FlowController::FlowController(modbus::base::ModbusMode mode)
    : mode_(mode) {}

void FlowController::markWritePending() {
    writeDrained_ = false;
    lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
}

void FlowController::markWriteDrained(std::chrono::steady_clock::time_point when) {
    writeDrained_ = true;
    lastWriteDrainedAt_ = when;
}

bool FlowController::isWriteDrained() const {
    return writeDrained_;
}

std::chrono::steady_clock::time_point FlowController::drainedAt() const {
    return lastWriteDrainedAt_;
}

void FlowController::updateRtuSendWindow(std::ptrdiff_t frameBytes,
                                         const modbus::base::ModbusConfig& config) {
    if (mode_ != base::ModbusMode::RTU) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();
    nextRtuSendAllowedAt_ = now
        + FrameExtractor::calculateFrameDuration(config, frameBytes)
        + FrameExtractor::calculateInterFrameDelay(config);
}

bool FlowController::isRtuSendWindowOpen(
    std::chrono::steady_clock::time_point now) const {
    if (mode_ != base::ModbusMode::RTU) {
        return true;
    }
    return now >= nextRtuSendAllowedAt_;
}

std::chrono::steady_clock::time_point FlowController::rtuSendWindowOpensAt() const {
    return nextRtuSendAllowedAt_;
}

void FlowController::reset() {
    writeDrained_ = true;
    lastWriteDrainedAt_ = std::chrono::steady_clock::time_point{};
    nextRtuSendAllowedAt_ = std::chrono::steady_clock::time_point{};
}

void FlowController::setMode(modbus::base::ModbusMode mode) {
    mode_ = mode;
    reset();
}

} // namespace modbus::session
