/**
 * @file RetryStrategy.h
 * @brief Header file for RetryStrategy.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "BackoffCalculator.h"
#include <chrono>

namespace modbus::session {

class RetryStrategy {
public:
    struct Config {
        int maxRetries = 0;
        BackoffCalculator::Config backoff;
    };

    explicit RetryStrategy(const Config& config);
    void reconfigure(const Config& config);

    [[nodiscard]] bool shouldRetry() const;
    [[nodiscard]] std::chrono::milliseconds nextWait() const;
    void recordAttempt();
    void reset();
    [[nodiscard]] int attemptCount() const;

private:
    Config config_;
    int attemptCount_ = 0;
    BackoffCalculator backoffCalculator_;
};

} // namespace modbus::session