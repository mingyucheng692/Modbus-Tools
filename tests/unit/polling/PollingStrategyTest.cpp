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
    PollingStrategy strategy;
    // 1000ms interval → 3000/1000 = 3
    EXPECT_EQ(strategy.calculateThreshold(1000), 3);
}

TEST(PollingStrategy, CalculateThresholdFastInterval) {
    PollingStrategy strategy;
    // 100ms interval → 3000/100 = 30, clamped to 10
    EXPECT_EQ(strategy.calculateThreshold(100), 10);
}

TEST(PollingStrategy, CalculateThresholdMediumInterval) {
    PollingStrategy strategy;
    // 500ms interval → 3000/500 = 6
    EXPECT_EQ(strategy.calculateThreshold(500), 6);
}

TEST(PollingStrategy, CalculateThresholdClampsMin) {
    PollingStrategy strategy;
    // Very slow interval → very low threshold, clamped to 3
    EXPECT_EQ(strategy.calculateThreshold(5000), 3);
}

TEST(PollingStrategy, ClassifyErrorTimeout) {
    PollingStrategy strategy;
    EXPECT_EQ(strategy.classifyError("connection timeout"), ErrorSeverity::Timeout);
    EXPECT_EQ(strategy.classifyError("request timed out"), ErrorSeverity::Timeout);
}

TEST(PollingStrategy, ClassifyErrorBusy) {
    PollingStrategy strategy;
    EXPECT_EQ(strategy.classifyError("device busy"), ErrorSeverity::Busy);
    EXPECT_EQ(strategy.classifyError("temporary failure"), ErrorSeverity::Busy);
}

TEST(PollingStrategy, ClassifyErrorNormal) {
    PollingStrategy strategy;
    EXPECT_EQ(strategy.classifyError("crc error"), ErrorSeverity::Normal);
    EXPECT_EQ(strategy.classifyError(""), ErrorSeverity::Normal);
}

TEST(PollingStrategy, EvaluateStateContinue) {
    PollingStrategy strategy;
    // 0 errors → continue
    EXPECT_EQ(strategy.evaluateState(0, 1000, false, 5), PollingDecision::Continue);
    // 1 error, not enough for escalation
    EXPECT_EQ(strategy.evaluateState(1, 500, false, 5), PollingDecision::Degraded);
}

TEST(PollingStrategy, EvaluateStateEscalatedByCount) {
    PollingStrategy strategy;
    // 5 consecutive errors, threshold is 5 → escalate
    EXPECT_EQ(strategy.evaluateState(5, 2000, false, 5), PollingDecision::Escalated);
}

TEST(PollingStrategy, EvaluateStateEscalatedByWindow) {
    PollingStrategy strategy;
    // 2 errors but over 3s window → escalate
    EXPECT_EQ(strategy.evaluateState(2, 3500, false, 5), PollingDecision::Escalated);
}

TEST(PollingStrategy, EvaluateStateEscalatedByZeroSuccess) {
    PollingStrategy strategy;
    // zeroSuccessWindow flag set → escalate regardless of count
    EXPECT_EQ(strategy.evaluateState(1, 500, true, 5), PollingDecision::Escalated);
}