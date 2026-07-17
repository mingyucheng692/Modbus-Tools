/**
 * @file ModbusPduBuilderTest.cpp
 * @brief Unit tests for modbus::base::pdu_builder free functions.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "modbus/base/ModbusPduBuilder.h"
#include "modbus/base/ModbusFrame.h"

using namespace modbus::base;

TEST(ModbusPduBuilderTest, BuildReadRequestBasic) {
    auto result = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, 0x006B, 3);
    ASSERT_TRUE(result.has_value()) << "Read request should succeed";
    EXPECT_EQ(result->functionCode(), FunctionCode::ReadHoldingRegisters);
    ASSERT_EQ(result->data().size(), 4);
}

TEST(ModbusPduBuilderTest, BuildReadRequestInvalidAddress) {
    auto result = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, 0x10000, 3);
    EXPECT_FALSE(result.has_value()) << "Out-of-range address should fail";
}

TEST(ModbusPduBuilderTest, BuildWriteRequestSingleCoilOn) {
    const QByteArray raw(reinterpret_cast<const char*>("\xFF\x00"), 2);
    auto result = pdu_builder::buildWriteRequest(FunctionCode::WriteSingleCoil, 0x00AC, 1, raw);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->functionCode(), FunctionCode::WriteSingleCoil);
    EXPECT_EQ(result->data().size(), 4);
    // Address: 0x00AC
    EXPECT_EQ(static_cast<uint8_t>(result->data()[0]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[1]), 0xAC);
    // Value: 0xFF00
    EXPECT_EQ(static_cast<uint8_t>(result->data()[2]), 0xFF);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[3]), 0x00);
}

TEST(ModbusPduBuilderTest, BuildWriteRequestSingleCoilOff) {
    const QByteArray raw(reinterpret_cast<const char*>("\x00\x00"), 2);
    auto result = pdu_builder::buildWriteRequest(FunctionCode::WriteSingleCoil, 0x0001, 1, raw);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->functionCode(), FunctionCode::WriteSingleCoil);
    // Value should be 0x0000
    EXPECT_EQ(static_cast<uint8_t>(result->data()[2]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[3]), 0x00);
}

TEST(ModbusPduBuilderTest, BuildWriteRequestSingleRegister) {
    const QByteArray raw(reinterpret_cast<const char*>("\x02\x58"), 2); // 600 decimal
    auto result = pdu_builder::buildWriteRequest(FunctionCode::WriteSingleRegister, 0x0001, 1, raw);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->functionCode(), FunctionCode::WriteSingleRegister);
    EXPECT_EQ(result->data().size(), 4);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[0]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[1]), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[2]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[3]), 0x58);
}

TEST(ModbusPduBuilderTest, BuildWriteRequestMultipleCoils) {
    // Set coils 0-9: 0xCD, 0x01 (binary: 1100_1101 0000_0001)
    const QByteArray raw(reinterpret_cast<const char*>("\xCD\x01"), 2);
    auto result = pdu_builder::buildWriteRequest(FunctionCode::WriteMultipleCoils, 0x0013, 10, raw);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->functionCode(), FunctionCode::WriteMultipleCoils);
    // Address 0x0013, quantity 10, byte count 2, data
    EXPECT_EQ(static_cast<uint8_t>(result->data()[0]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[1]), 0x13);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[2]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[3]), 0x0A);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[4]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[5]), 0xCD);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[6]), 0x01);
}

TEST(ModbusPduBuilderTest, BuildWriteRequestMultipleRegisters) {
    // Two registers: 0x022B, 0x0001
    const QByteArray raw(reinterpret_cast<const char*>("\x02\x2B\x00\x01"), 4);
    auto result = pdu_builder::buildWriteRequest(FunctionCode::WriteMultipleRegisters, 0x0001, 2, raw);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->functionCode(), FunctionCode::WriteMultipleRegisters);
    // Address 0x0001, quantity 2, byte count 4, data
    EXPECT_EQ(static_cast<uint8_t>(result->data()[0]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[1]), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[2]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[3]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[4]), 0x04);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[5]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[6]), 0x2B);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[7]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result->data()[8]), 0x01);
}

TEST(ModbusPduBuilderTest, BuildWriteRequestUnsupportedFunctionCode) {
    const QByteArray raw(reinterpret_cast<const char*>("\x00"), 1);
    QString error;
    auto result = pdu_builder::buildWriteRequest(
        static_cast<FunctionCode>(0x99), 0, 1, raw, &error);
    EXPECT_FALSE(result.has_value());
    EXPECT_FALSE(error.isEmpty());
}
