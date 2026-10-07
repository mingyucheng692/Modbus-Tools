// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <chrono>
#include <cstddef>
#include <map>

namespace common {

// Role: Suppresses repeated log events for the same key within a sliding time window.
// Thread-safety: Thread-compatible; single-threaded only (no internal synchronization).
// Ownership: Owned as a value member by worker/executor components (e.g. RequestExecutor).
template <typename Key>
class LogDedupe {
public:
    explicit LogDedupe(std::chrono::steady_clock::duration window)
        : window_(window) {}

    // Key: Integer-based tuple to avoid heap allocation; now: steady_clock monotonic time point.
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

    // Opportunistically evicts expired entries when map size exceeds threshold to bound memory.
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
