#include <gtest/gtest.h>
#include "modbus/base/ModbusDataHelper.h"

using namespace modbus::base;

TEST(ModbusDataHelperTest, ParseHex) {
    EXPECT_EQ(ModbusDataHelper::parseHex("01 23 AB"), QByteArray::fromHex("0123AB"));
    EXPECT_EQ(ModbusDataHelper::parseHex("1 23 AB"), QByteArray::fromHex("0123AB")); // Odd digit prepends 0
    EXPECT_EQ(ModbusDataHelper::parseHex("G1 23"), QByteArray::fromHex("0123")); // Ignore non-hex
}

TEST(ModbusDataHelperTest, ParseDecimalList) {
    bool ok = false;
    QByteArray result = ModbusDataHelper::parseDecimalList("123, 456; 789", ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(result.size(), 6);
    EXPECT_EQ(static_cast<unsigned char>(result[0]), 0x00);
    EXPECT_EQ(static_cast<unsigned char>(result[1]), 123);
    EXPECT_EQ(static_cast<unsigned char>(result[2]), 0x01);
    EXPECT_EQ(static_cast<unsigned char>(result[3]), 0xC8); // 456 = 0x01C8
    
    ModbusDataHelper::parseDecimalList("70000", ok);
    EXPECT_FALSE(ok);
}

TEST(ModbusDataHelperTest, ParseBinary) {
    QByteArray result = ModbusDataHelper::parseBinary("1 1 0 1 0 0 0 0 1");
    // Bits: 1(L), 1, 0, 1, 0, 0, 0, 0 -> 0x0B (0000 1011) ? Wait LSB-first
    // Bit 0: 1, 1: 1, 2: 0, 3: 1, 4: 0, 5: 0, 6: 0, 7: 0 -> 1+2+8 = 11 (0x0B)
    // Bit 8: 1 -> Byte 1: 0x01
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(static_cast<unsigned char>(result[0]), 0x0B);
    EXPECT_EQ(static_cast<unsigned char>(result[1]), 0x01);
}

TEST(ModbusDataHelperTest, ParseSmartInt) {
    bool ok = false;
    EXPECT_EQ(ModbusDataHelper::parseSmartInt("0x12", &ok), 0x12);
    EXPECT_TRUE(ok);
    
    EXPECT_EQ(ModbusDataHelper::parseSmartInt("12H", &ok), 0x12);
    EXPECT_TRUE(ok);
    
    EXPECT_EQ(ModbusDataHelper::parseSmartInt("18", &ok), 18);
    EXPECT_TRUE(ok);
    
    ModbusDataHelper::parseSmartInt("0x12H", &ok);
    EXPECT_FALSE(ok);
}
