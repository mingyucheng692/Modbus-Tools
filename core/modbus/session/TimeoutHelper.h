/**
 * @file TimeoutHelper.h
 * @brief Header-only timeout / abortable-wait utilities for session operations.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <algorithm>

namespace modbus::session {

/// Qt event-loop-friendly wait slice so aborted_ is polled at least every
/// 5 ms, preventing UI freezes during long wait periods.
constexpr auto kQtWaitSlice = std::chrono::milliseconds(5);

/// Wait for @p delay with abort support. Returns true if the delay expired
/// naturally, false if aborted_ was set to true during the wait.
inline bool waitForAbortableDelay(std::mutex& mutex,
                                  std::condition_variable& cv,
                                  std::atomic<bool>& aborted,
                                  std::chrono::steady_clock::duration delay) {
    if (delay <= std::chrono::steady_clock::duration::zero()) {
        return !aborted.load();
    }

    const auto deadline = std::chrono::steady_clock::now() + delay;
    while (!aborted.load() && std::chrono::steady_clock::now() < deadline) {
        const auto now = std::chrono::steady_clock::now();
        const auto remaining =
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice,
                                    std::max(std::chrono::milliseconds(1), remaining));
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait_for(lock, slice, [&aborted]() { return aborted.load(); });
    }
    return !aborted.load();
}

/// Wait for a predicate to become true or @p deadline to expire.
/// Returns true if the predicate became true, false on timeout.
template <typename Predicate>
inline bool waitForCondition(std::mutex& mutex,
                             std::condition_variable& cv,
                             Predicate predicate,
                             std::chrono::steady_clock::time_point deadline) {
    const auto now = std::chrono::steady_clock::now();
    const auto remaining =
        std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
    const auto slice = std::min(kQtWaitSlice,
                                std::max(std::chrono::milliseconds(1), remaining));
    std::unique_lock<std::mutex> lock(mutex);
    return cv.wait_for(lock, slice, predicate);
}

} // namespace modbus::session