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
#include <atomic>
#include <algorithm>
#include <thread>

namespace modbus::session {

/// Polling wait slice so aborted_ is checked at least every 5 ms.
constexpr auto kWaitSlice = std::chrono::milliseconds(5);


/// Wait for a predicate to become true or @p deadline to expire.
/// Returns true if the predicate became true, false on timeout.
template <typename Predicate>
inline bool waitForCondition(Predicate predicate,
                             std::chrono::steady_clock::time_point deadline) {
    while (std::chrono::steady_clock::now() < deadline) {
        if (predicate()) {
            return true;
        }
        const auto now = std::chrono::steady_clock::now();
        const auto remaining =
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        const auto slice = std::min(kWaitSlice,
                                    std::max(std::chrono::milliseconds(1), remaining));
        std::this_thread::sleep_for(slice);
    }
    return predicate();
}

} // namespace modbus::session