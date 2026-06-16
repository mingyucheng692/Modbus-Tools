/**
 * @file TimeoutController.cpp
 * @brief Implementation of TimeoutController.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "TimeoutController.h"
#include <algorithm>

namespace modbus::session {

TimeoutController::TimeoutController(std::mutex& mutex,
                                     std::condition_variable& cv,
                                     std::atomic<bool>& aborted)
    : mutex_(mutex), cv_(cv), aborted_(aborted) {}

bool TimeoutController::waitForAbortableDelay(std::chrono::steady_clock::duration delay) {
    if (delay <= std::chrono::steady_clock::duration::zero()) {
        return !aborted_.load();
    }

    const auto deadline = std::chrono::steady_clock::now() + delay;
    while (!aborted_.load() && std::chrono::steady_clock::now() < deadline) {
        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, slice, [this]() {
            return aborted_.load();
        });
    }
    return !aborted_.load();
}

} // namespace modbus::session