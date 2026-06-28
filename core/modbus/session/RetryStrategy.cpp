/**
 * @file RetryStrategy.cpp
 * @brief Implementation of RetryStrategy.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "RetryStrategy.h"

namespace modbus::session {

RetryStrategy::RetryStrategy(const Config& config)
    : config_(config)
    , backoffCalculator_(config.backoff)
{
}

void RetryStrategy::reconfigure(const Config& config)
{
    config_ = config;
    backoffCalculator_ = BackoffCalculator(config.backoff);
}

bool RetryStrategy::shouldRetry() const
{
    return attemptCount_ <= config_.maxRetries;
}

std::chrono::milliseconds RetryStrategy::nextWait() const
{
    const int attempt = (attemptCount_ > 0) ? (attemptCount_ - 1) : 0;
    return std::chrono::milliseconds(backoffCalculator_.calculateMs(attempt));
}

void RetryStrategy::recordAttempt()
{
    ++attemptCount_;
}

void RetryStrategy::reset()
{
    attemptCount_ = 0;
}

int RetryStrategy::attemptCount() const
{
    return attemptCount_;
}

} // namespace modbus::session