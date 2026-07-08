/**
 * @file ModbusResponseTest.cpp
 * @brief Unit tests for the ModbusResponse factories, focusing on the
 *        structured Busy error code introduced to distinguish soft-lock
 *        contention from genuine protocol/transport errors.
 */

#include <gtest/gtest.h>

#include "modbus/session/IModbusClient.h"

using modbus::session::ModbusResponse;
using modbus::session::ModbusResponseKind;
using modbus::session::RequestError;

TEST(ModbusResponseTest, Error_DefaultsToNoneErrorCode) {
    const auto response = ModbusResponse::Error("boom");
    EXPECT_EQ(response.kind, ModbusResponseKind::Error);
    EXPECT_TRUE(response.isError());
    EXPECT_EQ(response.errorCode, RequestError::None);
    EXPECT_FALSE(response.isBusy());
    EXPECT_EQ(response.error.toStdString(), "boom");
}

TEST(ModbusResponseTest, Busy_SetsBusyErrorCodeAndMessage) {
    const auto response = ModbusResponse::Busy("Request already in progress");
    EXPECT_EQ(response.kind, ModbusResponseKind::Error);
    EXPECT_TRUE(response.isError());
    EXPECT_EQ(response.errorCode, RequestError::Busy);
    EXPECT_TRUE(response.isBusy());
    EXPECT_EQ(response.error.toStdString(), "Request already in progress");
}

TEST(ModbusResponseTest, Success_HasNoBusyCode) {
    const auto response = ModbusResponse::Success(
        modbus::base::Pdu(modbus::base::FunctionCode::ReadHoldingRegisters, QByteArray(2, 0)),
        10);
    EXPECT_EQ(response.kind, ModbusResponseKind::Success);
    EXPECT_FALSE(response.isError());
    EXPECT_EQ(response.errorCode, RequestError::None);
    EXPECT_FALSE(response.isBusy());
}
