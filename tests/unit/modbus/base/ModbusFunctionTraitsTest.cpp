// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "modbus/base/ModbusFunctionTraits.h"
#include "modbus/base/ModbusTypes.h"
#include "Config.h"

using namespace modbus::base;
using namespace modbus::base::function_traits;

TEST(ModbusFunctionTraitsTest, ReadOperationIdentification) {
    EXPECT_TRUE(isReadOperation(FunctionCode::ReadCoils));
    EXPECT_TRUE(isReadOperation(FunctionCode::ReadDiscreteInputs));
    EXPECT_TRUE(isReadOperation(FunctionCode::ReadHoldingRegisters));
    EXPECT_TRUE(isReadOperation(FunctionCode::ReadInputRegisters));

    // uint8_t overloads
    EXPECT_TRUE(isReadOperation(static_cast<uint8_t>(0x01)));
    EXPECT_TRUE(isReadOperation(static_cast<uint8_t>(0x02)));
    EXPECT_TRUE(isReadOperation(static_cast<uint8_t>(0x03)));
    EXPECT_TRUE(isReadOperation(static_cast<uint8_t>(0x04)));

    // Write operations are not read
    EXPECT_FALSE(isReadOperation(FunctionCode::WriteSingleCoil));
    EXPECT_FALSE(isReadOperation(FunctionCode::WriteSingleRegister));
    EXPECT_FALSE(isReadOperation(FunctionCode::WriteMultipleCoils));
    EXPECT_FALSE(isReadOperation(FunctionCode::WriteMultipleRegisters));
    EXPECT_FALSE(isReadOperation(static_cast<uint8_t>(0x05)));
    EXPECT_FALSE(isReadOperation(static_cast<uint8_t>(0x06)));
    EXPECT_FALSE(isReadOperation(static_cast<uint8_t>(0x0F)));
    EXPECT_FALSE(isReadOperation(static_cast<uint8_t>(0x10)));
    EXPECT_FALSE(isReadOperation(static_cast<uint8_t>(0x90)));
}

TEST(ModbusFunctionTraitsTest, WriteOperationIdentification) {
    EXPECT_TRUE(isWriteOperation(FunctionCode::WriteSingleCoil));
    EXPECT_TRUE(isWriteOperation(FunctionCode::WriteSingleRegister));
    EXPECT_TRUE(isWriteOperation(FunctionCode::WriteMultipleCoils));
    EXPECT_TRUE(isWriteOperation(FunctionCode::WriteMultipleRegisters));

    // uint8_t overloads
    EXPECT_TRUE(isWriteOperation(static_cast<uint8_t>(0x05)));
    EXPECT_TRUE(isWriteOperation(static_cast<uint8_t>(0x06)));
    EXPECT_TRUE(isWriteOperation(static_cast<uint8_t>(0x0F)));
    EXPECT_TRUE(isWriteOperation(static_cast<uint8_t>(0x10)));

    // Single write checks
    EXPECT_TRUE(isSingleWrite(FunctionCode::WriteSingleCoil));
    EXPECT_TRUE(isSingleWrite(FunctionCode::WriteSingleRegister));
    EXPECT_TRUE(isSingleWrite(static_cast<uint8_t>(0x05)));
    EXPECT_TRUE(isSingleWrite(static_cast<uint8_t>(0x06)));
    EXPECT_FALSE(isSingleWrite(FunctionCode::WriteMultipleCoils));
    EXPECT_FALSE(isSingleWrite(static_cast<uint8_t>(0x10)));

    // Multiple write checks
    EXPECT_TRUE(isMultipleWrite(FunctionCode::WriteMultipleCoils));
    EXPECT_TRUE(isMultipleWrite(FunctionCode::WriteMultipleRegisters));
    EXPECT_TRUE(isMultipleWrite(static_cast<uint8_t>(0x0F)));
    EXPECT_TRUE(isMultipleWrite(static_cast<uint8_t>(0x10)));
    EXPECT_FALSE(isMultipleWrite(FunctionCode::WriteSingleCoil));
    EXPECT_FALSE(isMultipleWrite(static_cast<uint8_t>(0x06)));
}

TEST(ModbusFunctionTraitsTest, MaxAllowedQuantity) {
    EXPECT_EQ(maxAllowedQuantity(FunctionCode::ReadCoils), config::Modbus::kMaxReadBitsQuantity);
    EXPECT_EQ(maxAllowedQuantity(FunctionCode::ReadDiscreteInputs), config::Modbus::kMaxReadBitsQuantity);
    EXPECT_EQ(maxAllowedQuantity(FunctionCode::ReadHoldingRegisters), config::Modbus::kMaxReadQuantity);
    EXPECT_EQ(maxAllowedQuantity(FunctionCode::ReadInputRegisters), config::Modbus::kMaxReadQuantity);

    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0x01)), 2000);
    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0x02)), 2000);
    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0x03)), 125);
    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0x04)), 125);

    EXPECT_EQ(maxAllowedQuantity(FunctionCode::WriteSingleCoil), 1);
    EXPECT_EQ(maxAllowedQuantity(FunctionCode::WriteSingleRegister), 1);
    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0x05)), 1);
    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0x06)), 1);

    EXPECT_EQ(maxAllowedQuantity(FunctionCode::WriteMultipleCoils), config::Modbus::kMaxWriteCoilsQuantity);
    EXPECT_EQ(maxAllowedQuantity(FunctionCode::WriteMultipleRegisters), config::Modbus::kMaxWriteRegistersQuantity);
    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0x0F)), 1968);
    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0x10)), 123);

    // Unsupported / unknown code returns 0
    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0x08)), 0);
    EXPECT_EQ(maxAllowedQuantity(static_cast<uint8_t>(0xFF)), 0);
}

TEST(ModbusFunctionTraitsTest, BroadcastAllowedPerIec61158) {
    // Slave ID != 0 (Unicast) allows both read and write
    EXPECT_TRUE(isBroadcastAllowed(1, FunctionCode::ReadHoldingRegisters));
    EXPECT_TRUE(isBroadcastAllowed(1, static_cast<uint8_t>(0x03)));
    EXPECT_TRUE(isBroadcastAllowed(247, FunctionCode::WriteSingleRegister));
    EXPECT_TRUE(isBroadcastAllowed(247, static_cast<uint8_t>(0x06)));

    // Slave ID == 0 (Broadcast) forbids read operations per IEC 61158 Section 4.2
    EXPECT_FALSE(isBroadcastAllowed(0, FunctionCode::ReadCoils));
    EXPECT_FALSE(isBroadcastAllowed(0, FunctionCode::ReadDiscreteInputs));
    EXPECT_FALSE(isBroadcastAllowed(0, FunctionCode::ReadHoldingRegisters));
    EXPECT_FALSE(isBroadcastAllowed(0, FunctionCode::ReadInputRegisters));
    EXPECT_FALSE(isBroadcastAllowed(0, static_cast<uint8_t>(0x01)));
    EXPECT_FALSE(isBroadcastAllowed(0, static_cast<uint8_t>(0x02)));
    EXPECT_FALSE(isBroadcastAllowed(0, static_cast<uint8_t>(0x03)));
    EXPECT_FALSE(isBroadcastAllowed(0, static_cast<uint8_t>(0x04)));

    // Slave ID == 0 (Broadcast) allows write operations
    EXPECT_TRUE(isBroadcastAllowed(0, FunctionCode::WriteSingleCoil));
    EXPECT_TRUE(isBroadcastAllowed(0, FunctionCode::WriteSingleRegister));
    EXPECT_TRUE(isBroadcastAllowed(0, FunctionCode::WriteMultipleCoils));
    EXPECT_TRUE(isBroadcastAllowed(0, FunctionCode::WriteMultipleRegisters));
    EXPECT_TRUE(isBroadcastAllowed(0, static_cast<uint8_t>(0x05)));
    EXPECT_TRUE(isBroadcastAllowed(0, static_cast<uint8_t>(0x06)));
    EXPECT_TRUE(isBroadcastAllowed(0, static_cast<uint8_t>(0x0F)));
    EXPECT_TRUE(isBroadcastAllowed(0, static_cast<uint8_t>(0x10)));
}

TEST(ModbusFunctionTraitsTest, FunctionCodeNames) {
    EXPECT_STREQ(functionCodeName(FunctionCode::ReadCoils), "Read Coils");
    EXPECT_STREQ(functionCodeName(FunctionCode::ReadDiscreteInputs), "Read Discrete Inputs");
    EXPECT_STREQ(functionCodeName(FunctionCode::ReadHoldingRegisters), "Read Holding Registers");
    EXPECT_STREQ(functionCodeName(FunctionCode::ReadInputRegisters), "Read Input Registers");
    EXPECT_STREQ(functionCodeName(FunctionCode::WriteSingleCoil), "Write Single Coil");
    EXPECT_STREQ(functionCodeName(FunctionCode::WriteSingleRegister), "Write Single Register");
    EXPECT_STREQ(functionCodeName(FunctionCode::WriteMultipleCoils), "Write Multiple Coils");
    EXPECT_STREQ(functionCodeName(FunctionCode::WriteMultipleRegisters), "Write Multiple Registers");

    EXPECT_STREQ(functionCodeName(static_cast<uint8_t>(0x03)), "Read Holding Registers");
    EXPECT_STREQ(functionCodeName(static_cast<uint8_t>(0x10)), "Write Multiple Registers");
    EXPECT_STREQ(functionCodeName(static_cast<uint8_t>(0x99)), "Unknown");
}

