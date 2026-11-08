// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "modbus/base/ModbusProtocolChecks.h"
#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusCrc.h"
#include "modbus/base/ModbusLrc.h"

using namespace modbus::base;

namespace {

TEST(ModbusProtocolChecksTest, InspectRtuAdu_LengthCap) {
    // Exactly 256 bytes (1B slave + 1B FC + 252B payload + 2B CRC) with valid
    // CRC is structurally valid and must survive the size cap.
    QByteArray adu;
    adu.append(static_cast<char>(0x01));
    adu.append(static_cast<char>(0x03));
    adu.append(QByteArray(252, '\x00'));
    appendModbusRtuCrc(adu);
    ASSERT_EQ(adu.size(), 256);
    EXPECT_EQ(inspectRtuAdu(adu, nullptr), 256);

    // 257 bytes is physically invalid regardless of content.
    QByteArray oversized(257, '\x00');
    EXPECT_EQ(inspectRtuAdu(oversized, nullptr), -1);
}

TEST(ModbusProtocolChecksTest, InspectAsciiAdu_LengthCap) {
    // Max-size frame: ':' + 255B binary as hex (510 chars) + CRLF = 513.
    QByteArray binary;
    binary.append(static_cast<char>(0x01));
    binary.append(static_cast<char>(0x03));
    binary.append(QByteArray(252, '\x00'));
    const uint8_t lrc = calculateModbusAsciiLrc(binary);
    binary.append(static_cast<char>(lrc));
    ASSERT_EQ(binary.size(), 255);

    QByteArray maxSizeFrame;
    maxSizeFrame.append(':');
    maxSizeFrame.append(binary.toHex().toUpper());
    maxSizeFrame.append("\r\n");
    ASSERT_EQ(maxSizeFrame.size(), 513);
    EXPECT_EQ(inspectAsciiAdu(maxSizeFrame, nullptr), 513);

    // 515 chars (512 hex + ':' + CRLF) exceeds the cap -> malformed.
    QByteArray oversized;
    oversized.append(':');
    oversized.append(QByteArray(512, '0'));
    oversized.append("\r\n");
    EXPECT_EQ(inspectAsciiAdu(oversized, nullptr), -1);
}

TEST(ModbusProtocolChecksTest, ValidateResponsePdu_ExceptionCodeRange) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));

    // In-range exception (0x0A Gateway Path Unavailable) passes structure.
    const Pdu inRange(FunctionCode::ReadHoldingRegisters,
                      ExceptionCode::GatewayPathUnavailable);
    EXPECT_EQ(validateResponsePdu(request, inRange), QString());

    // Out-of-range exception code (0x0B) is rejected.
    const Pdu outOfRange(static_cast<FunctionCode>(0x83), QByteArray("\x0B", 1));
    EXPECT_FALSE(validateResponsePdu(request, outOfRange).isEmpty());
}

} // namespace
