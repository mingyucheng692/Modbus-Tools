// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "modbus/session/RequestValidator.h"
#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusTypes.h"
#include "modbus/base/ModbusConfig.h"

using namespace modbus::base;
using namespace modbus::session::request_validator;

TEST(RequestValidatorTest, AcceptsValidReadHoldingRegisters) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00100002"));
    const auto result = validate(request, 1, ModbusMode::TCP);
    EXPECT_TRUE(result.valid) << result.error.toStdString();
    EXPECT_TRUE(result.error.isEmpty());
}

TEST(RequestValidatorTest, RejectsInvalidRtuSlaveId) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00100002"));
    // Negative slave ID
    auto result = validate(request, -1, ModbusMode::RTU);
    EXPECT_FALSE(result.valid);
    EXPECT_FALSE(result.error.isEmpty());

    // Slave ID > 247
    result = validate(request, 248, ModbusMode::RTU);
    EXPECT_FALSE(result.valid);
    EXPECT_FALSE(result.error.isEmpty());
}

TEST(RequestValidatorTest, RejectsInvalidReadPayloadLength) {
    // Truncated payload (< 4 bytes)
    const Pdu shortRequest(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("001000"));
    auto result = validate(shortRequest, 1, ModbusMode::TCP);
    EXPECT_FALSE(result.valid);

    // Zero quantity
    const Pdu zeroQtyRequest(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00100000"));
    result = validate(zeroQtyRequest, 1, ModbusMode::TCP);
    EXPECT_FALSE(result.valid);
}

TEST(RequestValidatorTest, RejectsReadQuantityExceedingLimit) {
    // Read holding registers limit is 125 (0x007D)
    const Pdu tooManyRegisters(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("0000007E"));
    const auto result = validate(tooManyRegisters, 1, ModbusMode::TCP);
    EXPECT_FALSE(result.valid);
}

TEST(RequestValidatorTest, AcceptsValidWriteSingleCoilAndRejectsInvalidValue) {
    // Valid ON (0xFF00)
    const Pdu coilOn(FunctionCode::WriteSingleCoil, QByteArray::fromHex("0001FF00"));
    auto result = validate(coilOn, 1, ModbusMode::TCP);
    EXPECT_TRUE(result.valid);

    // Valid OFF (0x0000)
    const Pdu coilOff(FunctionCode::WriteSingleCoil, QByteArray::fromHex("00010000"));
    result = validate(coilOff, 1, ModbusMode::TCP);
    EXPECT_TRUE(result.valid);

    // Invalid value (0x0100)
    const Pdu invalidCoil(FunctionCode::WriteSingleCoil, QByteArray::fromHex("00010100"));
    result = validate(invalidCoil, 1, ModbusMode::TCP);
    EXPECT_FALSE(result.valid);
}

TEST(RequestValidatorTest, ValidatesWriteMultipleRegisters) {
    // Valid: Start=0, Qty=2, ByteCount=4, Data=4 bytes
    const Pdu validMulti(FunctionCode::WriteMultipleRegisters, QByteArray::fromHex("000000020412345678"));
    auto result = validate(validMulti, 1, ModbusMode::TCP);
    EXPECT_TRUE(result.valid);

    // Mismatched ByteCount: declared 4 bytes but provided only 2 bytes
    const Pdu shortData(FunctionCode::WriteMultipleRegisters, QByteArray::fromHex("00000002041234"));
    result = validate(shortData, 1, ModbusMode::TCP);
    EXPECT_FALSE(result.valid);
}
