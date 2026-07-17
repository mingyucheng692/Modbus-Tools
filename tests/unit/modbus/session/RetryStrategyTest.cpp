/**
 * @file RetryStrategyTest.cpp
 * @brief Unit tests for RetryStrategy.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "modbus/session/RetryStrategy.h"

using namespace modbus::session;

TEST(RetryStrategy, NoRetriesByDefault) {
    RetryStrategy::Config cfg;
    cfg.maxRetries = 0;
    RetryStrategy strategy(cfg);
    EXPECT_FALSE(strategy.shouldRetry());
}

TEST(RetryStrategy, ShouldRetryWhenAttemptsRemain) {
    RetryStrategy::Config cfg;
    cfg.maxRetries = 3;
    RetryStrategy strategy(cfg);
    EXPECT_TRUE(strategy.shouldRetry());
    strategy.recordAttempt();
    EXPECT_TRUE(strategy.shouldRetry());
    strategy.recordAttempt();
    strategy.recordAttempt();
    EXPECT_TRUE(strategy.shouldRetry());
    strategy.recordAttempt();
    EXPECT_FALSE(strategy.shouldRetry());
}

TEST(RetryStrategy, ResetClearsAttemptCount) {
    RetryStrategy::Config cfg;
    cfg.maxRetries = 2;
    RetryStrategy strategy(cfg);
    strategy.recordAttempt();
    strategy.recordAttempt();
    EXPECT_EQ(strategy.attemptCount(), 2);
    strategy.reset();
    EXPECT_EQ(strategy.attemptCount(), 0);
    EXPECT_TRUE(strategy.shouldRetry());
}

TEST(RetryStrategy, BackoffIncreasesWithAttempts) {
    RetryStrategy::Config cfg;
    cfg.maxRetries = 5;
    cfg.baseIntervalMs = 100;
    cfg.maxIntervalMs = 5000;
    cfg.backoffFactor = 2.0;
    cfg.jitterPercent = 0; // disable jitter for deterministic test

    // attempt 0: 100ms, attempt 1: 200ms, attempt 2: 400ms
    EXPECT_EQ(RetryStrategy::calculateBackoffMs(cfg, 0), 100);
    EXPECT_EQ(RetryStrategy::calculateBackoffMs(cfg, 1), 200);
    EXPECT_EQ(RetryStrategy::calculateBackoffMs(cfg, 2), 400);
}

TEST(RetryStrategy, BackoffCappedAtMaxInterval) {
    RetryStrategy::Config cfg;
    cfg.baseIntervalMs = 100;
    cfg.maxIntervalMs = 500;
    cfg.backoffFactor = 10.0;
    cfg.jitterPercent = 0;

    // 100 * 10^2 = 10000, capped to 500
    EXPECT_EQ(RetryStrategy::calculateBackoffMs(cfg, 2), 500);
}

TEST(RetryStrategy, JitterStaysWithinWindow) {
    RetryStrategy::Config cfg;
    cfg.baseIntervalMs = 1000;
    cfg.maxIntervalMs = 5000;
    cfg.backoffFactor = 1.0;
    cfg.jitterPercent = 20;

    // With 20% jitter on 1000ms, result should be in [800, 1200]
    for (int i = 0; i < 100; ++i) {
        const int delay = RetryStrategy::calculateBackoffMs(cfg, 0);
        EXPECT_GE(delay, 800);
        EXPECT_LE(delay, 1200);
    }
}

TEST(RetryStrategy, NegativeBaseIntervalSanitized) {
    RetryStrategy::Config cfg;
    cfg.baseIntervalMs = -100;
    cfg.maxIntervalMs = 5000;
    cfg.backoffFactor = 2.0;
    cfg.jitterPercent = 0;

    EXPECT_EQ(RetryStrategy::calculateBackoffMs(cfg, 0), 0);
}

TEST(RetryStrategy, NextWaitUsesAttemptMinusOne) {
    RetryStrategy::Config cfg;
    cfg.maxRetries = 3;
    cfg.baseIntervalMs = 100;
    cfg.maxIntervalMs = 5000;
    cfg.backoffFactor = 2.0;
    cfg.jitterPercent = 0;

    RetryStrategy strategy(cfg);
    // attemptCount_ = 0, nextWait uses attempt = 0 → 100ms
    EXPECT_EQ(strategy.nextWait().count(), 100);

    strategy.recordAttempt();
    // attemptCount_ = 1, nextWait uses attempt = 0 → 100ms
    EXPECT_EQ(strategy.nextWait().count(), 100);

    strategy.recordAttempt();
    // attemptCount_ = 2, nextWait uses attempt = 1 → 200ms
    EXPECT_EQ(strategy.nextWait().count(), 200);
}

TEST(RetryStrategy, ReconfigureUpdatesConfig) {
    RetryStrategy::Config cfg1;
    cfg1.maxRetries = 1;
    RetryStrategy strategy(cfg1);
    EXPECT_TRUE(strategy.shouldRetry());

    RetryStrategy::Config cfg2;
    cfg2.maxRetries = 0;
    strategy.reconfigure(cfg2);
    EXPECT_FALSE(strategy.shouldRetry());
}
