/**
 * @file test_modbus_frame_builder.cpp
 * @brief Unit tests for pure Modbus frame building functions (RTU, TCP, ASCII).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "modbus/base/ModbusAduBuilder.h"
#include "modbus/base/ModbusPduBuilder.h"
#include "modbus/base/ModbusCrc.h"
#include "modbus/base/ModbusLrc.h"
#include "modbus/base/ModbusTypes.h"

using namespace modbus::base;

TEST(ModbusFrameBuilderPureLogic, BuildRtuAdu_ReadHoldingRegisters_CorrectCrc) {
    const auto pduOpt = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, 0, 2);
    ASSERT_TRUE(pduOpt.has_value());

    const QByteArray adu = buildRtuAdu(1, *pduOpt);
    // Standard Modbus RTU frame for Read Holding Registers:
    // Slave(1) + FC(03) + StartAddr(00 00) + Quantity(00 02) + CRC(C4 0B)
    const QByteArray expected = QByteArray::fromHex("010300000002C40B");
    EXPECT_EQ(adu, expected);

    // Verify CRC independently
    ASSERT_GE(adu.size(), 2);
    const uint16_t computedCrc = calculateModbusRtuCrc(adu.left(adu.size() - 2));
    const auto lowByte = static_cast<uint8_t>(adu[adu.size() - 2]);
    const auto highByte = static_cast<uint8_t>(adu[adu.size() - 1]);
    const uint16_t storedCrc = static_cast<uint16_t>(lowByte | (highByte << 8));
    EXPECT_EQ(computedCrc, storedCrc);
}

TEST(ModbusFrameBuilderPureLogic, BuildTcpAdu_ReadHoldingRegisters_ValidMbapHeader) {
    const auto pduOpt = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, 0, 2);
    ASSERT_TRUE(pduOpt.has_value());

    const QByteArray adu = buildTcpAdu(1, *pduOpt, 0);
    // MBAP: TID(00 00) + PID(00 00) + Len(00 06) + UnitID(01) + PDU(03 00 00 00 02)
    const QByteArray expected = QByteArray::fromHex("000000000006010300000002");
    EXPECT_EQ(adu, expected);
}

TEST(ModbusFrameBuilderPureLogic, BuildTcpAdu_CustomTransactionId) {
    const auto pduOpt = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, 0, 2);
    ASSERT_TRUE(pduOpt.has_value());

    const QByteArray adu = buildTcpAdu(1, *pduOpt, 0x1234);
    // MBAP: TID(12 34) + PID(00 00) + Len(00 06) + UnitID(01) + PDU(03 00 00 00 02)
    const QByteArray expected = QByteArray::fromHex("123400000006010300000002");
    EXPECT_EQ(adu, expected);
}

TEST(ModbusFrameBuilderPureLogic, BuildAsciiAdu_ReadHoldingRegisters_ValidLrcAndDelimiters) {
    const auto pduOpt = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, 0, 2);
    ASSERT_TRUE(pduOpt.has_value());

    const QByteArray adu = buildAsciiAdu(1, *pduOpt);
    // Raw bytes: 01 03 00 00 00 02
    // Sum = 1 + 3 + 2 = 6. LRC = -6 (0xFA).
    // Hex: 010300000002FA
    // Full ASCII frame: :010300000002FA\r\n
    const QByteArray expected = ":010300000002FA\r\n";
    EXPECT_EQ(adu, expected);
    EXPECT_TRUE(adu.startsWith(':'));
    EXPECT_TRUE(adu.endsWith("\r\n"));
}

TEST(ModbusFrameBuilderPureLogic, BuildRtuAdu_WriteSingleRegister) {
    const auto pduOpt = pdu_builder::buildWriteSingleRegister(1, 0x1234);
    ASSERT_TRUE(pduOpt.has_value());

    const QByteArray adu = buildRtuAdu(1, *pduOpt);
    // Slave(01) + FC(06) + RegAddr(00 01) + Value(12 34) + CRC(2 bytes)
    EXPECT_EQ(adu.size(), 8);
    EXPECT_EQ(static_cast<uint8_t>(adu[0]), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(adu[1]), 0x06);
    EXPECT_EQ(static_cast<uint8_t>(adu[2]), 0x00);
    EXPECT_EQ(static_cast<uint8_t>(adu[3]), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(adu[4]), 0x12);
    EXPECT_EQ(static_cast<uint8_t>(adu[5]), 0x34);

    const uint16_t computedCrc = calculateModbusRtuCrc(adu.left(6));
    const auto lowByte = static_cast<uint8_t>(adu[6]);
    const auto highByte = static_cast<uint8_t>(adu[7]);
    const uint16_t storedCrc = static_cast<uint16_t>(lowByte | (highByte << 8));
    EXPECT_EQ(computedCrc, storedCrc);
}

TEST(ModbusFrameBuilderPureLogic, BuildRtuAdu_WriteMultipleRegisters_CorrectByteCount) {
    const QByteArray registerBytes = QByteArray::fromHex("000A000B");
    const auto pduOpt = pdu_builder::buildWriteMultipleRegisters(0, 2, registerBytes);
    ASSERT_TRUE(pduOpt.has_value());

    const QByteArray adu = buildRtuAdu(2, *pduOpt);
    // Slave(02) + FC(10) + Start(00 00) + Qty(00 02) + ByteCount(04) + Data(00 0A 00 0B) + CRC
    EXPECT_EQ(adu.size(), 13);
    EXPECT_EQ(static_cast<uint8_t>(adu[0]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(adu[1]), 0x10);
    EXPECT_EQ(static_cast<uint8_t>(adu[6]), 0x04); // Byte count = 4

    const uint16_t computedCrc = calculateModbusRtuCrc(adu.left(11));
    const auto lowByte = static_cast<uint8_t>(adu[11]);
    const auto highByte = static_cast<uint8_t>(adu[12]);
    const uint16_t storedCrc = static_cast<uint16_t>(lowByte | (highByte << 8));
    EXPECT_EQ(computedCrc, storedCrc);
}

TEST(ModbusFrameBuilderPureLogic, PduBuilder_InvalidParameters_ReturnsNullOpt) {
    QString error;
    // Quantity 0 is invalid
    auto pdu = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, 0, 0, &error);
    EXPECT_FALSE(pdu.has_value());
    EXPECT_FALSE(error.isEmpty());

    // Quantity > 2000 is invalid
    error.clear();
    pdu = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, 0, 2001, &error);
    EXPECT_FALSE(pdu.has_value());
    EXPECT_FALSE(error.isEmpty());

    // Negative address is invalid
    error.clear();
    pdu = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, -1, 10, &error);
    EXPECT_FALSE(pdu.has_value());
    EXPECT_FALSE(error.isEmpty());

    // Address > 0xFFFF is invalid
    error.clear();
    pdu = pdu_builder::buildReadRequest(FunctionCode::ReadHoldingRegisters, 0x10000, 10, &error);
    EXPECT_FALSE(pdu.has_value());
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusFrameBuilderPureLogic, AsciiAdu_LrcCalculation_MatchesSpec) {
    // Modbus specification standard example:
    // Slave 0x11, FC 0x03, Start 0x006B, Qty 0x0003
    // Sum = 0x11 + 0x03 + 0x00 + 0x6B + 0x00 + 0x03 = 0x82 (130)
    // Two's complement: 0x100 - 0x82 = 0x7E
    const QByteArray testData = QByteArray::fromHex("1103006B0003");
    const uint8_t lrc = calculateModbusAsciiLrc(testData);
    EXPECT_EQ(lrc, 0x7E);
}
