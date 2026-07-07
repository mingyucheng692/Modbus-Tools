/**
 * @file RetryStrategy.cpp
 * @brief Implementation of RetryStrategy.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "RetryStrategy.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace modbus::session {

RetryStrategy::RetryStrategy(const Config& config)
    : config_(config)
{
}

void RetryStrategy::reconfigure(const Config& config)
{
    config_ = config;
}

bool RetryStrategy::shouldRetry() const
{
    return attemptCount_ <= config_.maxRetries;
}

std::chrono::milliseconds RetryStrategy::nextWait() const
{
    const int attempt = (attemptCount_ > 0) ? (attemptCount_ - 1) : 0;
    return std::chrono::milliseconds(calculateBackoffMs(config_, attempt));
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

int RetryStrategy::sanitizeDelayMs(int value)
{
    return std::max(0, value);
}

int RetryStrategy::calculateBackoffMs(const Config& config, int attempt)
{
    const double factor = std::max(1.0, config.backoffFactor);
    const int sanitizedBase = sanitizeDelayMs(config.baseIntervalMs);
    const int sanitizedMax = std::max(sanitizedBase, sanitizeDelayMs(config.maxIntervalMs));
    const double scaled = static_cast<double>(sanitizedBase) * std::pow(factor, static_cast<double>(attempt));
    const int cappedDelay = std::min(sanitizedMax, static_cast<int>(std::llround(scaled)));

    const int jitterPercent = std::clamp(config.jitterPercent, 0, 100);
    if (jitterPercent == 0 || cappedDelay == 0) {
        return cappedDelay;
    }

    thread_local std::mt19937 generator{std::random_device{}()};
    const int jitterWindow = std::max(1, (cappedDelay * jitterPercent) / 100);
    std::uniform_int_distribution<int> distribution(-jitterWindow, jitterWindow);
    return std::max(0, cappedDelay + distribution(generator));
}

} // namespace modbus::session
