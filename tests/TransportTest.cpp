#include <gtest/gtest.h>
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
    }

    std::unique_ptr<ModbusTcpTransport> tcp_;
    std::unique_ptr<ModbusRtuTransport> rtu_;
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
