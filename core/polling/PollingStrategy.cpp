/**
 * @file PollingStrategy.cpp
 * @brief Implementation of PollingStrategy.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "PollingStrategy.h"
#include <algorithm>

namespace core::polling {

int PollingStrategy::calculateThreshold(int intervalMs) const {
    constexpr int kTargetUpgradeWindowMs = 3000;
    constexpr int kMinThreshold = 3;
    constexpr int kMaxThreshold = 10;

    const int effectiveInterval = std::max(intervalMs, 1);
    const int roundedThreshold = (kTargetUpgradeWindowMs + effectiveInterval / 2) / effectiveInterval;
    return std::clamp(roundedThreshold, kMinThreshold, kMaxThreshold);
}

ErrorSeverity PollingStrategy::classifyError(std::string_view errorText) const {
    if (errorText.find("timeout") != std::string_view::npos
        || errorText.find("timed out") != std::string_view::npos) {
        return ErrorSeverity::Timeout;
    }
    if (errorText.find("busy") != std::string_view::npos
        || errorText.find("tempor") != std::string_view::npos) {
        return ErrorSeverity::Busy;
    }
    return ErrorSeverity::Normal;
}

PollingDecision PollingStrategy::evaluateState(int consecutiveErrors,
                                                int failureStreakMs,
                                                bool zeroSuccessWindow,
                                                int threshold) const {
    constexpr int kZeroSuccessWindowMs = 3000;

    if (consecutiveErrors <= 0) {
        return PollingDecision::Continue;
    }

    const bool shouldEscalate = zeroSuccessWindow
        || (failureStreakMs >= kZeroSuccessWindowMs)
        || (consecutiveErrors >= threshold);

    if (shouldEscalate) {
        return PollingDecision::Escalated;
    }

    return PollingDecision::Degraded;
}

} // namespace core::polling