/**
 * @file BackoffCalculator.cpp
 * @brief Implementation of BackoffCalculator.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "BackoffCalculator.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace modbus::session {

int BackoffCalculator::sanitizeDelayMs(int value)
{
    return std::max(0, value);
}

BackoffCalculator::BackoffCalculator(const Config& config)
    : config_(config)
{
}

int BackoffCalculator::calculateMs(int attempt) const
{
    const double factor = std::max(1.0, config_.backoffFactor);
    const int sanitizedBase = sanitizeDelayMs(config_.baseIntervalMs);
    const int sanitizedMax = std::max(sanitizedBase, sanitizeDelayMs(config_.maxIntervalMs));
    const double scaled = static_cast<double>(sanitizedBase) * std::pow(factor, static_cast<double>(attempt));
    const int cappedDelay = std::min(sanitizedMax, static_cast<int>(std::llround(scaled)));

    const int jitterPercent = std::clamp(config_.jitterPercent, 0, 100);
    if (jitterPercent == 0 || cappedDelay == 0) {
        return cappedDelay;
    }

    thread_local std::mt19937 generator{std::random_device{}()};
    const int jitterWindow = std::max(1, (cappedDelay * jitterPercent) / 100);
    std::uniform_int_distribution<int> distribution(-jitterWindow, jitterWindow);
    return std::max(0, cappedDelay + distribution(generator));
}

} // namespace modbus::session