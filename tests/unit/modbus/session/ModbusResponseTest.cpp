// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "modbus/session/SessionTypes.h"

using modbus::session::ModbusResponse;
using modbus::session::ModbusResponseKind;
using modbus::base::ModbusError;

TEST(ModbusResponseTest, Error_DefaultsToNoneErrorCode) {
    const auto response = ModbusResponse::Error("boom");
    EXPECT_EQ(response.kind, ModbusResponseKind::Error);
    EXPECT_TRUE(response.isError());
    EXPECT_EQ(response.errorCode, ModbusError::None);
    EXPECT_FALSE(response.isBusy());
    EXPECT_EQ(response.error.toStdString(), "boom");
}

TEST(ModbusResponseTest, Busy_SetsBusyErrorCodeAndMessage) {
    const auto response = ModbusResponse::Busy("Request already in progress");
    EXPECT_EQ(response.kind, ModbusResponseKind::Error);
    EXPECT_TRUE(response.isError());
    EXPECT_EQ(response.errorCode, ModbusError::Busy);
    EXPECT_TRUE(response.isBusy());
    EXPECT_EQ(response.error.toStdString(), "Request already in progress");
}

TEST(ModbusResponseTest, Success_HasNoBusyCode) {
    const auto response = ModbusResponse::Success(
        modbus::base::Pdu(modbus::base::FunctionCode::ReadHoldingRegisters, QByteArray(2, 0)),
        10);
    EXPECT_EQ(response.kind, ModbusResponseKind::Success);
    EXPECT_FALSE(response.isError());
    EXPECT_EQ(response.errorCode, ModbusError::None);
    EXPECT_FALSE(response.isBusy());
}

TEST(ModbusResponseTest, StructuredModbusErrorFactory) {
    const auto response = ModbusResponse::Error(modbus::base::ModbusError::Timeout);
    EXPECT_EQ(response.kind, ModbusResponseKind::Error);
    EXPECT_TRUE(response.isError());
    EXPECT_EQ(response.errorCode, modbus::base::ModbusError::Timeout);
    EXPECT_EQ(response.error.toStdString(), "Timeout");

    const auto res = response.toResult();
    EXPECT_TRUE(res.isErr());
    EXPECT_FALSE(res.isOk());
    EXPECT_EQ(res.error(), modbus::base::ModbusError::Timeout);
}

TEST(ModbusResponseTest, SuccessToResultConvertsPdu) {
    const modbus::base::Pdu pdu(modbus::base::FunctionCode::ReadHoldingRegisters, QByteArray(4, 0x01));
    const auto response = ModbusResponse::Success(pdu, 25);
    const auto res = response.toResult();
    EXPECT_TRUE(res.isOk());
    EXPECT_FALSE(res.isErr());
    EXPECT_EQ(res.value().functionCode(), modbus::base::FunctionCode::ReadHoldingRegisters);
    EXPECT_EQ(res.value().data().size(), 4);
}

TEST(ModbusResponseTest, ResultTemplateBasicOperations) {
    using common::Result;
    using modbus::base::ModbusError;

    Result<int, ModbusError> okRes = Result<int, ModbusError>::ok(42);
    EXPECT_TRUE(okRes.isOk());
    EXPECT_FALSE(okRes.isErr());
    EXPECT_EQ(okRes.value(), 42);
    EXPECT_EQ(okRes.valueOr(0), 42);

    Result<int, ModbusError> errRes = Result<int, ModbusError>::err(ModbusError::CrcMismatch);
    EXPECT_FALSE(errRes.isOk());
    EXPECT_TRUE(errRes.isErr());
    EXPECT_EQ(errRes.error(), ModbusError::CrcMismatch);
    EXPECT_EQ(errRes.valueOr(100), 100);

    Result<void, ModbusError> voidOk = Result<void, ModbusError>::ok();
    EXPECT_TRUE(voidOk.isOk());
    EXPECT_FALSE(voidOk.isErr());

    Result<void, ModbusError> voidErr = Result<void, ModbusError>::err(ModbusError::WriteFailed);
    EXPECT_FALSE(voidErr.isOk());
    EXPECT_TRUE(voidErr.isErr());
    EXPECT_EQ(voidErr.error(), ModbusError::WriteFailed);
}

TEST(ModbusResponseTest, NoResponseExpectedToResultConvertsPdu) {
    const modbus::base::Pdu pdu(modbus::base::FunctionCode::WriteSingleRegister, QByteArray(4, 0x00));
    const auto response = ModbusResponse::NoResponseExpected(pdu, 1);
    EXPECT_TRUE(response.isNoResponseExpected());
    EXPECT_FALSE(response.isError());
    const auto res = response.toResult();
    EXPECT_TRUE(res.isOk());
    EXPECT_EQ(res.value().functionCode(), modbus::base::FunctionCode::WriteSingleRegister);
}

TEST(ModbusResponseTest, UnstructuredErrorToResultMapsToUnknown) {
    const auto response = ModbusResponse::Error(QStringLiteral("Unmapped legacy error string"));
    EXPECT_TRUE(response.isError());
    EXPECT_EQ(response.errorCode, modbus::base::ModbusError::None);
    const auto res = response.toResult();
    EXPECT_TRUE(res.isErr());
    EXPECT_EQ(res.error(), modbus::base::ModbusError::Unknown);
}

TEST(ModbusResponseTest, FromResultRoundTrip) {
    using modbus::base::ModbusError;
    using modbus::base::Pdu;
    using modbus::base::FunctionCode;

    const Pdu pdu(FunctionCode::ReadCoils, QByteArray(1, 0x01));
    const auto okRes = common::Result<Pdu, ModbusError>::ok(pdu);
    const auto respOk = ModbusResponse::fromResult(okRes, 42, 1);
    EXPECT_FALSE(respOk.isError());
    EXPECT_EQ(respOk.rttMs, 42);
    EXPECT_EQ(respOk.attemptCount, 1);
    EXPECT_EQ(respOk.retryCount(), 0);
    EXPECT_EQ(respOk.pdu.functionCode(), FunctionCode::ReadCoils);

    const auto errRes = common::Result<Pdu, ModbusError>::err(ModbusError::ConnectionFailed);
    const auto respErr = ModbusResponse::fromResult(errRes, -1, 2);
    EXPECT_TRUE(respErr.isError());
    EXPECT_EQ(respErr.errorCode, ModbusError::ConnectionFailed);
    EXPECT_EQ(respErr.attemptCount, 2);
    EXPECT_EQ(respErr.retryCount(), 1);
    EXPECT_EQ(respErr.toResult().error(), ModbusError::ConnectionFailed);
}

