#include <gtest/gtest.h>
#include "modbus/base/ModbusFrame.h"

using namespace modbus::base;

TEST(ModbusPduTest, NormalPdu) {
    QByteArray data = QByteArray::fromHex("00000001");
    Pdu pdu(FunctionCode::ReadHoldingRegisters, data);
    
    EXPECT_EQ(pdu.functionCode(), FunctionCode::ReadHoldingRegisters);
    EXPECT_EQ(pdu.data(), data);
    EXPECT_FALSE(pdu.isException());
    EXPECT_EQ(pdu.originalFunctionCode(), FunctionCode::ReadHoldingRegisters);
    
    QByteArray serialized = pdu.toByteArray();
    EXPECT_EQ(serialized.size(), 5);
    EXPECT_EQ(static_cast<uint8_t>(serialized[0]), 0x03);
    EXPECT_EQ(serialized.mid(1), data);
}

TEST(ModbusPduTest, ExceptionPdu) {
    Pdu pdu(FunctionCode::ReadHoldingRegisters, ExceptionCode::IllegalDataAddress);
    
    EXPECT_TRUE(pdu.isException());
    EXPECT_EQ(pdu.originalFunctionCode(), FunctionCode::ReadHoldingRegisters);
    // 0x03 | 0x80 = 0x83
    EXPECT_EQ(static_cast<uint8_t>(pdu.functionCode()), 0x83);
    EXPECT_EQ(pdu.exceptionCode(), ExceptionCode::IllegalDataAddress);
    
    QByteArray serialized = pdu.toByteArray();
    ASSERT_EQ(serialized.size(), 2);
    EXPECT_EQ(static_cast<uint8_t>(serialized[0]), 0x83);
    EXPECT_EQ(static_cast<uint8_t>(serialized[1]), 0x02);
}

TEST(ModbusPduTest, DefaultPdu) {
    Pdu pdu;
    // Default in code is ReadHoldingRegisters (0x03)
    EXPECT_EQ(pdu.functionCode(), FunctionCode::ReadHoldingRegisters);
    EXPECT_TRUE(pdu.data().isEmpty());
}
