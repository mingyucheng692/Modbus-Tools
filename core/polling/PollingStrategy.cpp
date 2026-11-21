// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "PollingStrategy.h"
#include <algorithm>

namespace core::polling {

int calculateThreshold(int intervalMs) {
    constexpr int kTargetUpgradeWindowMs = 3000;
    constexpr int kMinThreshold = 3;
    constexpr int kMaxThreshold = 10;

    const int effectiveInterval = std::max(intervalMs, 1);
    const int roundedThreshold = (kTargetUpgradeWindowMs + effectiveInterval / 2) / effectiveInterval;
    return std::clamp(roundedThreshold, kMinThreshold, kMaxThreshold);
}

ErrorSeverity classifyError(::modbus::base::ModbusError error) {
    if (error == ::modbus::base::ModbusError::Timeout) {
        return ErrorSeverity::Timeout;
    }
    if (error == ::modbus::base::ModbusError::Busy) {
        return ErrorSeverity::Busy;
    }
    return ErrorSeverity::Normal;
}

ErrorSeverity classifyError(std::string_view errorText) {
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

PollingDecision evaluateState(int consecutiveErrors,
                              int failureStreakMs,
                              int threshold,
                              ErrorSeverity severity) {
    constexpr int kZeroSuccessWindowMs = 3000;

    if (consecutiveErrors <= 0) {
        return PollingDecision::Continue;
    }

    int effectiveThreshold = threshold;
    if (severity == ErrorSeverity::Timeout) {
        effectiveThreshold = std::max(2, threshold - 1);
    } else if (severity == ErrorSeverity::Busy) {
        effectiveThreshold = std::min(10, threshold + 1);
    }

    const bool shouldEscalate = (failureStreakMs >= kZeroSuccessWindowMs)
        || (consecutiveErrors >= effectiveThreshold);

    if (shouldEscalate) {
        return PollingDecision::Escalated;
    }

    return PollingDecision::Degraded;
}

} // namespace core::polling
