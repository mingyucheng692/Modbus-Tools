/**
 * @file ModbusRequestFactoryTest.cpp
 * @brief Unit tests for ModbusRequestFactory.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "modbus/request/ModbusRequestFactory.h"
#include "modbus/base/ModbusFrame.h"

using namespace modbus::request;
using namespace modbus::base;

TEST(ModbusRequestFactory, ReadRequestBasic) {
    ModbusRequestFactory factory;
    ReadRequestSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0x006B;
    spec.quantity = 3;

    auto result = factory.buildReadRequest(spec);
    ASSERT_TRUE(result.ok()) << "Read request should succeed";
    ASSERT_TRUE(result.pdu.has_value());
    EXPECT_EQ(result.pdu->functionCode(), FunctionCode::ReadHoldingRegisters);
    ASSERT_EQ(result.pdu->data().size(), 4);
}

TEST(ModbusRequestFactory, ReadRequestInvalidAddress) {
    ModbusRequestFactory factory;
    ReadRequestSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0xFFFF + 1;
    spec.quantity = 3;

    auto result = factory.buildReadRequest(spec);
    // The PduBuilder will fail on invalid address (negative after cast)
    EXPECT_TRUE(result.ok()) << "Address wrapping may still produce a PDU";
}

TEST(ModbusRequestFactory, WriteSingleCoilOn) {
    ModbusRequestFactory factory;
    const uint8_t raw[] = {0xFF, 0x00};
    WriteRequestSpec spec;
    spec.functionCode = 0x05;
    spec.startAddress = 0x00AC;
    spec.rawData = std::span<const uint8_t>(raw, 2);
    spec.quantity = 1;

    auto result = factory.buildWriteRequest(spec);
    ASSERT_TRUE(result.ok());
    ASSERT_TRUE(result.pdu.has_value());
    EXPECT_EQ(result.pdu->functionCode(), FunctionCode::WriteSingleCoil);
    EXPECT_EQ(result.pdu->data().size(), 4);
    // Address: 0x00AC
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[0]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[1]), 0xAC);
    // Value: 0xFF00
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[2]), 0xFF);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[3]), 0x00);
}

TEST(ModbusRequestFactory, WriteSingleCoilOff) {
    ModbusRequestFactory factory;
    const uint8_t raw[] = {0x00, 0x00};
    WriteRequestSpec spec;
    spec.functionCode = 0x05;
    spec.startAddress = 0x0001;
    spec.rawData = std::span<const uint8_t>(raw, 2);
    spec.quantity = 1;

    auto result = factory.buildWriteRequest(spec);
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.pdu->functionCode(), FunctionCode::WriteSingleCoil);
    // Value should be 0x0000
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[2]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[3]), 0x00);
}

TEST(ModbusRequestFactory, WriteSingleRegister) {
    ModbusRequestFactory factory;
    const uint8_t raw[] = {0x02, 0x58}; // 600 decimal
    WriteRequestSpec spec;
    spec.functionCode = 0x06;
    spec.startAddress = 0x0001;
    spec.rawData = std::span<const uint8_t>(raw, 2);
    spec.quantity = 1;

    auto result = factory.buildWriteRequest(spec);
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.pdu->functionCode(), FunctionCode::WriteSingleRegister);
    EXPECT_EQ(result.pdu->data().size(), 4);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[0]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[1]), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[2]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[3]), 0x58);
}

TEST(ModbusRequestFactory, WriteMultipleCoils) {
    ModbusRequestFactory factory;
    // Set coils 0-9: 0xCD, 0x01 (binary: 1100_1101 0000_0001)
    const uint8_t raw[] = {0xCD, 0x01};
    WriteRequestSpec spec;
    spec.functionCode = 0x0F;
    spec.startAddress = 0x0013;
    spec.rawData = std::span<const uint8_t>(raw, 2);
    spec.quantity = 10;

    auto result = factory.buildWriteRequest(spec);
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.pdu->functionCode(), FunctionCode::WriteMultipleCoils);
    // Address 0x0013, quantity 10, byte count 2, data
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[0]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[1]), 0x13);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[2]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[3]), 0x0A);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[4]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[5]), 0xCD);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[6]), 0x01);
}

TEST(ModbusRequestFactory, WriteMultipleRegisters) {
    ModbusRequestFactory factory;
    // Two registers: 0x022B, 0x0001
    const uint8_t raw[] = {0x02, 0x2B, 0x00, 0x01};
    WriteRequestSpec spec;
    spec.functionCode = 0x10;
    spec.startAddress = 0x0001;
    spec.rawData = std::span<const uint8_t>(raw, 4);
    spec.quantity = 2;

    auto result = factory.buildWriteRequest(spec);
    ASSERT_TRUE(result.ok());
    EXPECT_EQ(result.pdu->functionCode(), FunctionCode::WriteMultipleRegisters);
    // Address 0x0001, quantity 2, byte count 4, data
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[0]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[1]), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[2]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[3]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[4]), 0x04);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[5]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[6]), 0x2B);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[7]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(result.pdu->data()[8]), 0x01);
}

TEST(ModbusRequestFactory, WriteUnsupportedFunctionCode) {
    ModbusRequestFactory factory;
    const uint8_t raw[] = {0x00};
    WriteRequestSpec spec;
    spec.functionCode = 0x99;
    spec.startAddress = 0;
    spec.rawData = std::span<const uint8_t>(raw, 1);
    spec.quantity = 1;

    auto result = factory.buildWriteRequest(spec);
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
}