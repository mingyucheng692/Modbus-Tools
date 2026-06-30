#include <gtest/gtest.h>
#include "modbus/transport/ModbusAsciiTransport.h"
#include "modbus/transport/ModbusTcpTransport.h"
#include "modbus/transport/ModbusRtuTransport.h"
#include "modbus/base/ModbusCrc.h"
#include <QCoreApplication>

using namespace modbus::transport;
using namespace modbus::base;

class TransportTest : public ::testing::Test {
protected:
    void SetUp() override {
        // TCP Transport instance
        tcp_ = std::make_unique<ModbusTcpTransport>();
        // RTU Transport instance
        rtu_ = std::make_unique<ModbusRtuTransport>();
        ascii_ = std::make_unique<ModbusAsciiTransport>();
    }

    std::unique_ptr<ModbusTcpTransport> tcp_;
    std::unique_ptr<ModbusRtuTransport> rtu_;
    std::unique_ptr<ModbusAsciiTransport> ascii_;
};

TEST_F(TransportTest, TcpBuildRequest) {
    Pdu pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    QByteArray adu = tcp_->buildRequest(pdu, 1);
    
    // TID (2 bytes, 00 01 because it's first) + PID (00 00) + LEN (00 06) + UnitID (01) + FC (03) + Data (00 00 00 01)
    // 6 bytes header + PDU (1 + 4 bytes) = 12 bytes total?
    // Wait, TID is managed internally by the transport? 
    // Actually ModbusTcpTransport likely increments a counter.
    
    EXPECT_EQ(adu.size(), 12);
    EXPECT_EQ(static_cast<uint8_t>(adu[6]), 1); // Unit ID
    EXPECT_EQ(static_cast<uint8_t>(adu[7]), 3); // Function Code
}

TEST_F(TransportTest, TcpIntegrityCheck) {
    // Correct MBAP + PDU (TID=1, PID=0, LEN=6, Unit=1, FC=3, ADDR=0000, QTY=0001)
    QByteArray fullAdu = QByteArray::fromHex("000100000006010300000001");
    EXPECT_EQ(tcp_->checkIntegrity(fullAdu), 12);
    
    // Partial MBAP
    EXPECT_EQ(tcp_->checkIntegrity(fullAdu.first(4)), 0);
    
    // Incomplete PDU (MBAP says 6 but we have less)
    EXPECT_EQ(tcp_->checkIntegrity(fullAdu.first(10)), 0);
    
    // Garbage (PID != 0)
    QByteArray garbage = QByteArray::fromHex("000100010006010300000001");
    EXPECT_EQ(tcp_->checkIntegrity(garbage), -1);
}

TEST_F(TransportTest, TcpParseResponse_MatchingTransaction_ReturnsParsedPdu) {
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    QByteArray requestAdu = tcp_->buildRequest(request, 1);

    QByteArray responseAdu;
    responseAdu.append(requestAdu.first(2));  // transaction id
    responseAdu.append("\x00\x00", 2);        // protocol id
    responseAdu.append("\x00\x05", 2);        // unit + fc + byteCount + 2 data bytes
    responseAdu.append('\x01');               // unit id
    responseAdu.append('\x03');               // function code
    responseAdu.append('\x02');               // byte count
    responseAdu.append('\x00');
    responseAdu.append('\x7B');

    ParseResponseResult result = tcp_->parseResponse(responseAdu);
    ASSERT_EQ(result.status, ParseResponseStatus::Ok);
    ASSERT_TRUE(result.pdu.has_value());
    EXPECT_EQ(result.pdu->functionCode(), FunctionCode::ReadHoldingRegisters);
    EXPECT_EQ(result.pdu->data(), QByteArray::fromHex("02007B"));
}

TEST_F(TransportTest, TcpResetPendingState_MakesOutstandingResponseUnmatched) {
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    QByteArray requestAdu = tcp_->buildRequest(request, 1);
    tcp_->resetPendingState();

    QByteArray responseAdu;
    responseAdu.append(requestAdu.first(2));  // transaction id
    responseAdu.append("\x00\x00", 2);        // protocol id
    responseAdu.append("\x00\x05", 2);        // unit + fc + byteCount + 2 data bytes
    responseAdu.append('\x01');               // unit id
    responseAdu.append('\x03');               // function code
    responseAdu.append('\x02');               // byte count
    responseAdu.append('\x00');
    responseAdu.append('\x7B');

    ParseResponseResult result = tcp_->parseResponse(responseAdu);
    EXPECT_EQ(result.status, ParseResponseStatus::Unmatched);
    EXPECT_FALSE(result.pdu.has_value());
}

TEST_F(TransportTest, RtuBuildRequest) {
    Pdu pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    QByteArray adu = rtu_->buildRequest(pdu, 1);
    
    // Slave(1) + FC(03) + Data(00 00 00 01) + CRC(2 bytes) = 8 bytes
    EXPECT_EQ(adu.size(), 8);
    EXPECT_EQ(static_cast<uint8_t>(adu[0]), 1);
    EXPECT_EQ(static_cast<uint8_t>(adu[1]), 3);
    
    // Verify CRC (LSB first)
    // For 01 03 00 00 00 01, the CRC calculated by the code is 0x0A84
    EXPECT_EQ(static_cast<uint8_t>(adu[6]), 0x84);
    EXPECT_EQ(static_cast<uint8_t>(adu[7]), 0x0A);
}

TEST_F(TransportTest, RtuIntegrityCheck) {
    // 01 03 02 00 7B -> CRC is 0x6738? Wait, let's check code output.
    // If we use 01 03 02 00 7B and expect it to pass, we need the matching CRC.
    QByteArray pduPart = QByteArray::fromHex("010302007B");
    uint16_t crc = calculateModbusRtuCrc(pduPart);
    QByteArray fullAdu = pduPart;
    fullAdu.append(static_cast<char>(crc & 0xFF));
    fullAdu.append(static_cast<char>((crc >> 8) & 0xFF));

    EXPECT_EQ(rtu_->checkIntegrity(fullAdu), fullAdu.size());
    
    // Partial
    EXPECT_EQ(rtu_->checkIntegrity(fullAdu.first(4)), 0);
    
    // CRC Mismatch
    QByteArray badCrc = QByteArray::fromHex("010302007B3868");
    EXPECT_EQ(rtu_->checkIntegrity(badCrc), -1);
}

TEST_F(TransportTest, RtuResetPendingState_MakesOutstandingResponseUnmatched) {
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    rtu_->buildRequest(request, 1);
    rtu_->resetPendingState();

    QByteArray pduPart = QByteArray::fromHex("010302007B");
    uint16_t crc = calculateModbusRtuCrc(pduPart);
    QByteArray responseAdu = pduPart;
    responseAdu.append(static_cast<char>(crc & 0xFF));
    responseAdu.append(static_cast<char>((crc >> 8) & 0xFF));

    ParseResponseResult result = rtu_->parseResponse(responseAdu);
    EXPECT_EQ(result.status, ParseResponseStatus::Unmatched);
    EXPECT_FALSE(result.pdu.has_value());
}

TEST_F(TransportTest, AsciiBuildRequest) {
    Pdu pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    QByteArray adu = ascii_->buildRequest(pdu, 1);

    EXPECT_EQ(adu, QByteArray(":010300000001FB\r\n"));
}

TEST_F(TransportTest, AsciiIntegrityCheck) {
    const QByteArray fullAdu(":010302007BFD\r\n");
    EXPECT_EQ(ascii_->checkIntegrity(fullAdu), fullAdu.size());
    EXPECT_EQ(ascii_->checkIntegrity(QByteArray(":010302007BFD")), 0);
    EXPECT_EQ(ascii_->checkIntegrity(QByteArray(":010302007BFC\r\n")), -1);
}

TEST_F(TransportTest, AsciiParseResponse_MatchingSlave_ReturnsParsedPdu) {
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    ascii_->buildRequest(request, 1);

    ParseResponseResult result = ascii_->parseResponse(QByteArray(":010302007BFD\r\n"));
    ASSERT_EQ(result.status, ParseResponseStatus::Ok);
    ASSERT_TRUE(result.pdu.has_value());
    EXPECT_EQ(result.pdu->functionCode(), FunctionCode::ReadHoldingRegisters);
    EXPECT_EQ(result.pdu->data(), QByteArray::fromHex("02007B"));
}
