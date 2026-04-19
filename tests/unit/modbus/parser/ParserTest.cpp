#include <gtest/gtest.h>
#include "modbus/parser/ModbusFrameParser.h"
#include <QDateTime>

using namespace modbus::core::parser;
using namespace modbus::base;

class FrameParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Preparation if needed
    }
};

TEST_F(FrameParserTest, ParseRtuResponseSuccess) {
    // Slave 1, FC 3, ByteCount 4, Val1=0x007B (123), Val2=0x01C8 (456), Correct CRC for 010304007B01C8 is 0x2C8A -> 8A 2C
    QByteArray frame = QByteArray::fromHex("010304007B01C88A2C");
    
    ParseResult result = ModbusFrameParser::parse(frame, ProtocolType::Rtu, 40001, 2);
    
    EXPECT_TRUE(result.isValid) << "Error: " << result.error.toStdString();
    EXPECT_EQ(result.protocol, ProtocolType::Rtu);
    EXPECT_EQ(result.type, FrameType::Response);
    EXPECT_EQ(result.slaveId, 1);
    EXPECT_EQ(result.functionCode, FunctionCode::ReadHoldingRegisters);
    EXPECT_TRUE(result.checksumValid);
    
    ASSERT_EQ(result.dataItems.size(), 2);
    EXPECT_EQ(result.dataItems[0].address, 40001);
    EXPECT_EQ(result.dataItems[0].value.toUInt(), 123);
    EXPECT_EQ(result.dataItems[1].address, 40002);
    EXPECT_EQ(result.dataItems[1].value.toUInt(), 456);
}

TEST_F(FrameParserTest, ParseRtuCrcError) {
    // Wrong CRC
    QByteArray frame = QByteArray::fromHex("010304007B01C88A2D");
    
    ParseResult result = ModbusFrameParser::parse(frame, ProtocolType::Rtu);
    
    EXPECT_FALSE(result.checksumValid);
    EXPECT_FALSE(result.isValid);
}

TEST_F(FrameParserTest, ParseTcpRequestSuccess) {
    // TID=0x0001, PID=0x0000, Len=0x0006, Slave=0x01, FC=0x03, Addr=0x0000, Qty=0x0001
    QByteArray frame = QByteArray::fromHex("000100000006010300000001");
    
    ParseResult result = ModbusFrameParser::parse(frame, ProtocolType::Tcp);
    
    EXPECT_TRUE(result.isValid);
    EXPECT_EQ(result.protocol, ProtocolType::Tcp);
    EXPECT_EQ(result.type, FrameType::Request);
    EXPECT_EQ(result.transactionId, 1);
    EXPECT_EQ(result.slaveId, 1);
    EXPECT_EQ(result.functionCode, FunctionCode::ReadHoldingRegisters);
}

TEST_F(FrameParserTest, ParseExceptionResponse) {
    // Slave 1, FC 0x83 (Error FC3), Exception 0x02 -> CRC 0xF1C0 -> C0 F1
    QByteArray frame = QByteArray::fromHex("018302C0F1");
    
    ParseResult result = ModbusFrameParser::parse(frame, ProtocolType::Rtu);
    
    EXPECT_TRUE(result.isValid) << "Error: " << result.error.toStdString();
    EXPECT_TRUE(result.isException);
    EXPECT_EQ(result.type, FrameType::Exception);
    EXPECT_EQ(result.exceptionCode, ExceptionCode::IllegalDataAddress);
}

TEST_F(FrameParserTest, ParsePduLinkToAnalyzer) {
    // Simulated PDU from Link to Analyzer: FC 3, ByteCount 2, Data 0x007B
    QByteArray pdu = QByteArray::fromHex("0302007B");
    ParseResult result;
    result.isValid = true;
    result.protocol = ProtocolType::Tcp; // Assume TCP context
    
    ModbusFrameParser::parsePdu(result, pdu, 40010, 1);
    
    EXPECT_EQ(result.functionCode, FunctionCode::ReadHoldingRegisters);
    ASSERT_EQ(result.dataItems.size(), 1);
    EXPECT_EQ(result.dataItems[0].address, 40010);
    EXPECT_EQ(result.dataItems[0].value.toUInt(), 123);
}

TEST_F(FrameParserTest, ByteOrderTransformation) {
    // 32-bit data spread across 2 registers: 0x12 0x34 0x56 0x78
    // In Modbus registers: 40001=0x1234, 40002=0x5678
    
    // Test ABCD (Big Endian)
    // 01 03 04 12 34 56 78 -> CRC 0x0781 -> 81 07
    ParseResult resABCD = ModbusFrameParser::parse(QByteArray::fromHex("010304123456788107"), ProtocolType::Rtu, 40001, 2, false, RegisterOrder::ABCD);
    EXPECT_TRUE(resABCD.isValid) << "Error: " << resABCD.error.toStdString();
    
    // Address 40001 val=0x1234
    ASSERT_GE(resABCD.dataItems.size(), 1);
    EXPECT_EQ(resABCD.dataItems[0].value.toUInt(), 0x1234);
    
    // Test DCBA (Little Endian Byte Swap)
    ParseResult resDCBA = resABCD;
    ModbusFrameParser::applyRegisterOrder(resDCBA, RegisterOrder::DCBA);
    // Original: R1=0x1234, R2=0x5678
    // DCBA: R1 gets swapped(R2) = 0x7856, R2 gets swapped(R1) = 0x3412
    ASSERT_EQ(resDCBA.dataItems.size(), 2);
    EXPECT_EQ(resDCBA.dataItems[0].value.toUInt(), 0x7856);
    EXPECT_EQ(resDCBA.dataItems[1].value.toUInt(), 0x3412);
}
