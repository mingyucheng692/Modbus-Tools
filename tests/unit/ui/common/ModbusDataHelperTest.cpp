// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "ui/common/ModbusDataHelper.h"

using namespace ui::common;

TEST(ModbusDataHelperTest, ParseHex) {
    EXPECT_EQ(data_helper::parseHex("01 23 AB"), QByteArray::fromHex("0123AB"));
    EXPECT_EQ(data_helper::parseHex("1 23 AB"), QByteArray::fromHex("0123AB"));
    EXPECT_EQ(data_helper::parseHex("G1 23"), QByteArray::fromHex("0123"));
}

TEST(ModbusDataHelperTest, ParseDecimalList) {
    bool ok = false;
    QByteArray result = data_helper::parseDecimalList("123, 456; 789", ok);
    EXPECT_TRUE(ok);
    EXPECT_EQ(result.size(), 6);
    EXPECT_EQ(static_cast<unsigned char>(result[0]), 0x00);
    EXPECT_EQ(static_cast<unsigned char>(result[1]), 123);
    EXPECT_EQ(static_cast<unsigned char>(result[2]), 0x01);
    EXPECT_EQ(static_cast<unsigned char>(result[3]), 0xC8);
    
    data_helper::parseDecimalList("70000", ok);
    EXPECT_FALSE(ok);
}

TEST(ModbusDataHelperTest, ParseBinary) {
    QByteArray result = data_helper::parseBinary("1 1 0 1 0 0 0 0 1");
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(static_cast<unsigned char>(result[0]), 0x0B);
    EXPECT_EQ(static_cast<unsigned char>(result[1]), 0x01);
}

TEST(ModbusDataHelperTest, ParseSmartInt) {
    bool ok = false;
    EXPECT_EQ(data_helper::parseSmartInt("0x12", &ok), 0x12);
    EXPECT_TRUE(ok);
    
    EXPECT_EQ(data_helper::parseSmartInt("12H", &ok), 0x12);
    EXPECT_TRUE(ok);
    
    EXPECT_EQ(data_helper::parseSmartInt("18", &ok), 18);
    EXPECT_TRUE(ok);
    
    data_helper::parseSmartInt("0x12H", &ok);
    EXPECT_FALSE(ok);
}
