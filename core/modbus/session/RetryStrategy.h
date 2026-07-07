/**
 * @file RetryStrategy.h
 * @brief Header file for RetryStrategy.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <chrono>

namespace modbus::session {

class RetryStrategy {
public:
    struct Config {
        int maxRetries = 0;
        int baseIntervalMs = 100;
        int maxIntervalMs = 5000;
        double backoffFactor = 2.0;
        int jitterPercent = 20;
    };

    explicit RetryStrategy(const Config& config);
    void reconfigure(const Config& config);

    [[nodiscard]] bool shouldRetry() const;
    [[nodiscard]] std::chrono::milliseconds nextWait() const;
    void recordAttempt();
    void reset();
    [[nodiscard]] int attemptCount() const;

    /// Calculate backoff delay for @p attempt using @p config without
    /// constructing a full RetryStrategy. Used by ConnectionManager for
    /// reconnection delays that don't need retry-counting semantics.
    [[nodiscard]] static int calculateBackoffMs(const Config& config, int attempt);

private:
    static int sanitizeDelayMs(int value);

    Config config_;
    int attemptCount_ = 0;
};

} // namespace modbus::session
