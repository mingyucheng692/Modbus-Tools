// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <cstdint>
#include <string_view>

namespace core::polling {

enum class ErrorSeverity {
    Normal,
    Timeout,
    Busy,
    Fatal
};

enum class PollingDecision {
    Continue,
    Degraded,
    Escalated,
    Recover
};

// Pure-logic strategy determining error escalation thresholds and polling state transitions.

// Calculates escalation threshold clamped to [3, 10]; faster intervals tolerate more errors.
int calculateThreshold(int intervalMs);

// Classifies error severity: timeout and busy are transient, others are normal or fatal.
ErrorSeverity classifyError(std::string_view errorText);

// Evaluates polling transition decision based on error count, failure streak duration, and threshold.
PollingDecision evaluateState(int consecutiveErrors,
                              int failureStreakMs,
                              bool zeroSuccessWindow,
                              int threshold);

} // namespace core::polling
