/**
 * @file PollingStrategyTest.cpp
 * @brief Unit tests for PollingStrategy.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "polling/PollingStrategy.h"

using namespace core::polling;

TEST(PollingStrategy, CalculateThresholdSlowInterval) {
    // 1000ms interval → 3000/1000 = 3
    EXPECT_EQ(calculateThreshold(1000), 3);
}

TEST(PollingStrategy, CalculateThresholdFastInterval) {
    // 100ms interval → 3000/100 = 30, clamped to 10
    EXPECT_EQ(calculateThreshold(100), 10);
}

TEST(PollingStrategy, CalculateThresholdMediumInterval) {
    // 500ms interval → 3000/500 = 6
    EXPECT_EQ(calculateThreshold(500), 6);
}

TEST(PollingStrategy, CalculateThresholdClampsMin) {
    // Very slow interval → very low threshold, clamped to 3
    EXPECT_EQ(calculateThreshold(5000), 3);
}

TEST(PollingStrategy, ClassifyErrorTimeout) {
    EXPECT_EQ(classifyError("connection timeout"), ErrorSeverity::Timeout);
    EXPECT_EQ(classifyError("request timed out"), ErrorSeverity::Timeout);
}

TEST(PollingStrategy, ClassifyErrorBusy) {
    EXPECT_EQ(classifyError("device busy"), ErrorSeverity::Busy);
    EXPECT_EQ(classifyError("temporary failure"), ErrorSeverity::Busy);
}

TEST(PollingStrategy, ClassifyErrorNormal) {
    EXPECT_EQ(classifyError("crc error"), ErrorSeverity::Normal);
    EXPECT_EQ(classifyError(""), ErrorSeverity::Normal);
}

TEST(PollingStrategy, EvaluateStateContinue) {
    // 0 errors → continue
    EXPECT_EQ(evaluateState(0, 1000, false, 5), PollingDecision::Continue);
    // 1 error, not enough for escalation
    EXPECT_EQ(evaluateState(1, 500, false, 5), PollingDecision::Degraded);
}

TEST(PollingStrategy, EvaluateStateEscalatedByCount) {
    // 5 consecutive errors, threshold is 5 → escalate
    EXPECT_EQ(evaluateState(5, 2000, false, 5), PollingDecision::Escalated);
}

TEST(PollingStrategy, EvaluateStateEscalatedByWindow) {
    // 2 errors but over 3s window → escalate
    EXPECT_EQ(evaluateState(2, 3500, false, 5), PollingDecision::Escalated);
}

TEST(PollingStrategy, EvaluateStateEscalatedByZeroSuccess) {
    // zeroSuccessWindow flag set → escalate regardless of count
    EXPECT_EQ(evaluateState(1, 500, true, 5), PollingDecision::Escalated);
}
