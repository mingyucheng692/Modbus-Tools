/**
 * @file LogDedupe.h
 * @brief Single-threaded, lock-free log deduplication helper.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <chrono>
#include <cstddef>
#include <map>

namespace common {

/**
 * @brief Suppresses repeated log events for the same key within a time window.
 *
 * @tparam Key A cheap, orderable, integer-based key (e.g. std::tuple of
 *         uint8_t). Deliberately NOT QString — keys must be hashable/comparable
 *         without heap allocation or formatting so that shouldLog() stays a pure
 *         in-memory operation even when called while holding a lock.
 *
 * @thread Thread-compatible: MUST be used from a single thread. No mutex or
 *         atomic is used by design — adding one would turn this into a global
 *         lock hotspot. The owning RequestExecutor runs all request handling on
 *         the dedicated worker thread, so single-threaded use is guaranteed.
 *
 * Usage:
 * @code
 *   if (dedupe.shouldLog(key, std::chrono::steady_clock::now())) {
 *       spdlog::warn(...);   // first occurrence or window expired -> full level
 *   } else {
 *       spdlog::debug(...);  // duplicate within window -> demoted
 *   }
 * @endcode
 */
template <typename Key>
class LogDedupe {
public:
    explicit LogDedupe(std::chrono::steady_clock::duration window)
        : window_(window) {}

    /**
     * @brief Returns true when the event should be logged at full severity.
     *
     * Pure memory operation: one map lookup + one duration comparison + at most
     * one map insert. Performs NO string construction or formatting.
     *
     * @param key  Deduplication key (integer tuple).
     * @param now  Current steady_clock time. Use steady_clock, never wall clock —
     *             a system time change must not corrupt the suppression window.
     */
    bool shouldLog(const Key& key, std::chrono::steady_clock::time_point now) {
        auto it = lastSeen_.find(key);
        if (it == lastSeen_.end() || (now - it->second) >= window_) {
            lastSeen_[key] = now;
            return true;
        }
        return false;
    }

    /// Drop all recorded keys (e.g. on session reset).
    void clear() { lastSeen_.clear(); }

    /**
     * @brief Opportunistically evict expired entries once the map grows past
     *        @p threshold, bounding memory under long error storms.
     */
    void prune(std::chrono::steady_clock::time_point now, std::size_t threshold) {
        if (lastSeen_.size() <= threshold) {
            return;
        }
        const auto cutoff = now - window_;
        for (auto it = lastSeen_.begin(); it != lastSeen_.end();) {
            if (it->second < cutoff) {
                it = lastSeen_.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    std::chrono::steady_clock::duration window_;
    std::map<Key, std::chrono::steady_clock::time_point> lastSeen_;
};

} // namespace common
