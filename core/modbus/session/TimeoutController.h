/**
 * @file TimeoutController.h
 * @brief Timeout and abort-aware wait utilities for Modbus session operations.
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
#include <functional>

namespace modbus::session {

/**
 * @brief Encapsulates timeout-based wait logic with abort support.
 *
 * @note This class must not outlive the referenced mutex, condition_variable,
 *       and aborted flag. The caller is responsible for ensuring proper lifetime
 *       ordering.
 *
 * @thread Safe to call from any thread, as long as the referenced
 *        synchronization primitives are properly shared.
 */
class TimeoutController {
public:
    static constexpr auto kQtWaitSlice = std::chrono::milliseconds(5);

    TimeoutController(std::mutex& mutex,
                      std::condition_variable& cv,
                      std::atomic<bool>& aborted);

    /**
     * @brief Wait for a predicate to become true or deadline to expire.
     * @tparam Predicate Callable returning bool
     * @param predicate Condition to check under the mutex
     * @param deadline Absolute time point after which to stop waiting
     * @return true if predicate became true, false if deadline expired
     */
    template<typename Predicate>
    bool waitForCondition(Predicate predicate,
                          std::chrono::steady_clock::time_point deadline) {
        const auto now = std::chrono::steady_clock::now();
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kQtWaitSlice, std::max(std::chrono::milliseconds(1), remaining));
        std::unique_lock<std::mutex> lock(mutex_);
        return cv_.wait_for(lock, slice, predicate);
    }

    /**
     * @brief Wait for a duration with abort support.
     * @param delay Duration to wait
     * @return true if delay completed, false if aborted
     */
    bool waitForAbortableDelay(std::chrono::steady_clock::duration delay);

private:
    std::mutex& mutex_;
    std::condition_variable& cv_;
    std::atomic<bool>& aborted_;
};

} // namespace modbus::session