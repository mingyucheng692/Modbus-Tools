/**
 * @file ReconnectPolicyTest.cpp
 * @brief Unit tests for ReconnectPolicy.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "ReconnectPolicy.h"

using io::ReconnectPolicy;

TEST(ReconnectPolicy, DefaultConfigHasThreeRetriesAndThreeSecondDelay) {
    ReconnectPolicy policy;
    EXPECT_EQ(policy.maxRetries(), 3);
    EXPECT_EQ(policy.delayMs(), 3000);
    EXPECT_EQ(policy.attemptCount(), 0);
}

TEST(ReconnectPolicy, ShouldRetryWhileAttemptsRemain) {
    ReconnectPolicy policy(3, 1000);
    EXPECT_TRUE(policy.shouldRetry());
    policy.onFailed();
    EXPECT_TRUE(policy.shouldRetry());
    policy.onFailed();
    EXPECT_TRUE(policy.shouldRetry());
    policy.onFailed();
    // attemptCount == maxRetries → no more retries
    EXPECT_FALSE(policy.shouldRetry());
}

TEST(ReconnectPolicy, ExhaustedWhenAttemptsReachMax) {
    ReconnectPolicy policy(2, 500);
    EXPECT_FALSE(policy.exhausted());
    policy.onFailed();
    EXPECT_FALSE(policy.exhausted());
    policy.onFailed();
    EXPECT_TRUE(policy.exhausted());
}

TEST(ReconnectPolicy, OnFailedStopsIncrementingAtMax) {
    ReconnectPolicy policy(2, 500);
    policy.onFailed();
    policy.onFailed();
    EXPECT_EQ(policy.attemptCount(), 2);
    // Beyond maxRetries, onFailed is a no-op
    policy.onFailed();
    EXPECT_EQ(policy.attemptCount(), 2);
}

TEST(ReconnectPolicy, OnSuccessResetsCounter) {
    ReconnectPolicy policy(3, 1000);
    policy.onFailed();
    policy.onFailed();
    EXPECT_EQ(policy.attemptCount(), 2);
    policy.onSuccess();
    EXPECT_EQ(policy.attemptCount(), 0);
    EXPECT_TRUE(policy.shouldRetry());
}

TEST(ReconnectPolicy, ResetClearsCounter) {
    ReconnectPolicy policy(3, 1000);
    policy.onFailed();
    policy.onFailed();
    EXPECT_EQ(policy.attemptCount(), 2);
    policy.reset();
    EXPECT_EQ(policy.attemptCount(), 0);
    EXPECT_FALSE(policy.exhausted());
}

TEST(ReconnectPolicy, NextDelayAlwaysReturnsFixedDelay) {
    ReconnectPolicy policy(5, 2500);
    // ReconnectPolicy uses fixed delay (no exponential backoff)
    EXPECT_EQ(policy.nextDelayMs(), 2500);
    policy.onFailed();
    EXPECT_EQ(policy.nextDelayMs(), 2500);
    policy.onFailed();
    EXPECT_EQ(policy.nextDelayMs(), 2500);
}

TEST(ReconnectPolicy, SetMaxRetriesUpdatesBehavior) {
    ReconnectPolicy policy(5, 1000);
    EXPECT_TRUE(policy.shouldRetry());
    policy.setMaxRetries(1);
    EXPECT_EQ(policy.maxRetries(), 1);
    EXPECT_TRUE(policy.shouldRetry());
    policy.onFailed();
    EXPECT_FALSE(policy.shouldRetry());
    EXPECT_TRUE(policy.exhausted());
}

TEST(ReconnectPolicy, SetDelayMsUpdatesNextDelay) {
    ReconnectPolicy policy(3, 1000);
    EXPECT_EQ(policy.nextDelayMs(), 1000);
    policy.setDelayMs(5000);
    EXPECT_EQ(policy.delayMs(), 5000);
    EXPECT_EQ(policy.nextDelayMs(), 5000);
}

TEST(ReconnectPolicy, MaxRetriesZeroDisablesRetriesButNotExhausted) {
    // maxRetries <= 0 is treated as "disabled": shouldRetry returns false,
    // but exhausted() also returns false (its contract requires maxRetries > 0).
    // onFailed still increments attemptCount_ (unbounded counter, no cap).
    ReconnectPolicy policy(0, 1000);
    EXPECT_FALSE(policy.shouldRetry());
    EXPECT_FALSE(policy.exhausted());
    policy.onFailed();
    EXPECT_FALSE(policy.shouldRetry());
    EXPECT_FALSE(policy.exhausted());
    EXPECT_EQ(policy.attemptCount(), 1);
}
