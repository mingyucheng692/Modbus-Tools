/**
 * @file PollingStrategy.h
 * @brief Pure-logic polling error escalation strategy — no Qt dependency.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

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

/**
 * @brief Pure-logic strategy extracted from PollingController.
 *
 * Determines error escalation thresholds and evaluates polling state
 * transitions based on consecutive error counts, failure streaks, and
 * error text classification.
 */

/**
 * @brief Calculate the escalation threshold based on polling interval.
 *
 * A faster polling interval yields a higher threshold (more errors
 * tolerated before escalation), clamped to [3, 10].
 *
 * @param intervalMs Polling interval in milliseconds.
 * @return Number of consecutive errors before escalation.
 */
int calculateThreshold(int intervalMs);

/**
 * @brief Classify an error message by severity.
 *
 * Timeout errors are treated as less severe (connection may recover).
 * Busy/temporary errors are treated as transient.
 *
 * @param errorText Lowercase error message text.
 * @return Error severity classification.
 */
ErrorSeverity classifyError(std::string_view errorText);

/**
 * @brief Evaluate the polling state transition decision.
 *
 * @param consecutiveErrors Number of consecutive failures.
 * @param failureStreakMs Duration of the current failure streak in milliseconds.
 * @param zeroSuccessWindow True if the entire failure streak window has zero successes.
 * @param threshold Consecutive error threshold for escalation.
 * @return The recommended polling decision.
 */
PollingDecision evaluateState(int consecutiveErrors,
                              int failureStreakMs,
                              bool zeroSuccessWindow,
                              int threshold);

} // namespace core::polling