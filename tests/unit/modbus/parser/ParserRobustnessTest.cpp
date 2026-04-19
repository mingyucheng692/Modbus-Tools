#include <gtest/gtest.h>
#include "modbus/parser/ModbusFrameParser.h"
#include <QByteArray>

using namespace modbus::core::parser;
using namespace modbus::base;

TEST(ParserRobustnessTest, TruncatedFrames) {
    // Only 3 bytes (too short for any valid ADU)
    QByteArray shortFrame = QByteArray::fromHex("010300");
    ParseResult result = ModbusFrameParser::parse(shortFrame, ProtocolType::Rtu);
    EXPECT_FALSE(result.isValid);
    EXPECT_TRUE(result.error.contains("too short", Qt::CaseInsensitive) || result.error.contains("invalid", Qt::CaseInsensitive));

    // Only MBAP header (6 bytes) for TCP, but missing PDU
    QByteArray tcpHeaderOnly = QByteArray::fromHex("000100000006");
    ParseResult resultTcp = ModbusFrameParser::parse(tcpHeaderOnly, ProtocolType::Tcp);
    EXPECT_FALSE(resultTcp.isValid);
}

TEST(ParserRobustnessTest, InvalidMBAPLength) {
    // TID=1, PID=0, Len=255 (too long/incorrect), Slave=1, FC=3...
    QByteArray badLenFrame = QByteArray::fromHex("0001000000FF010300000001");
    ParseResult result = ModbusFrameParser::parse(badLenFrame, ProtocolType::Tcp);
    EXPECT_FALSE(result.isValid);
}

TEST(ParserRobustnessTest, ProtocolViolation) {
    // TCP frame but PID is not 0
    QByteArray badPidFrame = QByteArray::fromHex("000100010006010300000001");
    ParseResult result = ModbusFrameParser::parse(badPidFrame, ProtocolType::Tcp);
    EXPECT_FALSE(result.isValid);
    
    // RTU frame but CRC is definitely wrong
    QByteArray badCrcFrame = QByteArray::fromHex("010304007B01C8FFFF");
    ParseResult resultRtu = ModbusFrameParser::parse(badCrcFrame, ProtocolType::Rtu);
    EXPECT_FALSE(resultRtu.isValid);
    EXPECT_FALSE(resultRtu.checksumValid);
}

TEST(ParserRobustnessTest, FuzzLikeInput) {
    // Random garbage bytes
    QByteArray garbage(300, 0xCC);
    ParseResult result = ModbusFrameParser::parse(garbage, ProtocolType::Unknown);
    EXPECT_FALSE(result.isValid);
}

TEST(ParserRobustnessTest, ExceptionResponseHandling) {
    // Slave 1, FC 0x83, Error 0x02, CRC C0 F1
    QByteArray excFrame = QByteArray::fromHex("018302C0F1");
    ParseResult result = ModbusFrameParser::parse(excFrame, ProtocolType::Rtu);
    
    EXPECT_TRUE(result.isValid);
    EXPECT_TRUE(result.isException);
    EXPECT_EQ(result.exceptionCode, ExceptionCode::IllegalDataAddress);
}
