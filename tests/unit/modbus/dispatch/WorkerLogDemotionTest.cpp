/**
 * @file WorkerLogDemotionTest.cpp
 * @brief Unit tests for the log-demotion predicate: clean successes
 *        (no error, no retry) go to debug; everything else keeps info.
 */

#include <gtest/gtest.h>

#include "modbus/dispatch/ModbusWorker.h"

using modbus::base::FunctionCode;
using modbus::base::Pdu;
using modbus::dispatch::isCleanSuccess;
using modbus::session::ModbusResponse;

namespace {

Pdu dummyPdu() {
    return Pdu(FunctionCode::ReadHoldingRegisters, QByteArray(2, 0));
}

}  // namespace

TEST(WorkerLogDemotionTest, SuccessWithoutRetryIsDemoted) {
    // attemptCount=1 → retryCount()==0 → clean success → debug
    EXPECT_TRUE(isCleanSuccess(ModbusResponse::Success(dummyPdu(), 10, /*attemptCount=*/1)));
}

TEST(WorkerLogDemotionTest, SuccessAfterRetryKeepsInfo) {
    // attemptCount=2 → one retry happened → must stay visible at info
    EXPECT_FALSE(isCleanSuccess(ModbusResponse::Success(dummyPdu(), 10, /*attemptCount=*/2)));
    EXPECT_FALSE(isCleanSuccess(ModbusResponse::Success(dummyPdu(), 10, /*attemptCount=*/5)));
}

TEST(WorkerLogDemotionTest, ErrorKeepsInfo) {
    EXPECT_FALSE(isCleanSuccess(ModbusResponse::Error(QStringLiteral("timeout"))));
    EXPECT_FALSE(isCleanSuccess(ModbusResponse::Error(QStringLiteral("timeout"), /*attemptCount=*/3)));
}

TEST(WorkerLogDemotionTest, BusyKeepsInfo) {
    EXPECT_FALSE(isCleanSuccess(ModbusResponse::Busy(QStringLiteral("Request already in progress"))));
}

TEST(WorkerLogDemotionTest, NoResponseExpectedFollowsSameRule) {
    // Write-only function codes: no response is not an error.
    EXPECT_TRUE(isCleanSuccess(ModbusResponse::NoResponseExpected(dummyPdu(), /*attemptCount=*/1)));
    EXPECT_FALSE(isCleanSuccess(ModbusResponse::NoResponseExpected(dummyPdu(), /*attemptCount=*/2)));
}

TEST(WorkerLogDemotionTest, BoundaryZeroAttemptCountClampsToClean) {
    // attemptCount=0 → retryCount() clamps to 0 → still a clean success
    EXPECT_TRUE(isCleanSuccess(ModbusResponse::Success(dummyPdu())));
}
