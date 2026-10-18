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

TEST(ModbusDataHelperTest, ConvertPastedText) {
    // Hex to Dec
    auto decVal = data_helper::convertPastedText("0x12", 10);
    ASSERT_TRUE(decVal.has_value());
    EXPECT_EQ(*decVal, "18");

    decVal = data_helper::convertPastedText("12H", 10);
    ASSERT_TRUE(decVal.has_value());
    EXPECT_EQ(*decVal, "18");

    // Dec to Hex
    auto hexVal = data_helper::convertPastedText("18", 16);
    ASSERT_TRUE(hexVal.has_value());
    EXPECT_EQ(*hexVal, "12");

    hexVal = data_helper::convertPastedText("0x3E8", 16);
    ASSERT_TRUE(hexVal.has_value());
    EXPECT_EQ(*hexVal, "3E8");

    // Multi-token list
    auto multiDec = data_helper::convertPastedText("0x0A, 0x14", 10);
    ASSERT_TRUE(multiDec.has_value());
    EXPECT_EQ(*multiDec, "10 20");

    // Invalid formats
    EXPECT_FALSE(data_helper::convertPastedText("0x12H", 10).has_value());
    EXPECT_FALSE(data_helper::convertPastedText("70000", 10).has_value()); // Overflow
    EXPECT_FALSE(data_helper::convertPastedText("", 10).has_value());
    EXPECT_FALSE(data_helper::convertPastedText("invalid", 10).has_value());
}

TEST(ModbusDataHelperTest, ConvertRadix) {
    // Single values
    EXPECT_EQ(data_helper::convertRadix("1000", 10, 16), "3E8");
    EXPECT_EQ(data_helper::convertRadix("3E8", 16, 10), "1000");
    EXPECT_EQ(data_helper::convertRadix("0x3E8", 16, 10), "1000");
    EXPECT_EQ(data_helper::convertRadix("3E8H", 16, 10), "1000");

    // Multi-value list
    EXPECT_EQ(data_helper::convertRadix("10 20 30", 10, 16), "A 14 1E");
    EXPECT_EQ(data_helper::convertRadix("0A, 14, 1E", 16, 10), "10 20 30");

    // Same radix returns trimmed
    EXPECT_EQ(data_helper::convertRadix("  1000  ", 10, 10), "1000");

    // Invalid / overflow preserves original
    EXPECT_EQ(data_helper::convertRadix("70000", 10, 16), "70000");
    EXPECT_EQ(data_helper::convertRadix("ZZZ", 16, 10), "ZZZ");
}

TEST(ModbusDataHelperTest, PackCoilBits_GoldenVectorAndInvariants) {
    // Golden vector: 110100001 (9 bits) -> 0x0B, 0x01 per IEC 61158 LSB-first
    auto res = data_helper::packCoilBits("110100001", 9);
    ASSERT_TRUE(res.isOk());
    ASSERT_EQ(res.value().size(), 2);
    EXPECT_EQ(static_cast<uint8_t>(res.value()[0]), 0x0B);
    EXPECT_EQ(static_cast<uint8_t>(res.value()[1]), 0x01);

    // With delimiters
    auto resDelim = data_helper::packCoilBits("1 1 0 1 0 0 0 0 1", 9);
    ASSERT_TRUE(resDelim.isOk());
    EXPECT_EQ(resDelim.value(), res.value());

    // Single bit
    auto res1 = data_helper::packCoilBits("1", 1);
    ASSERT_TRUE(res1.isOk());
    ASSERT_EQ(res1.value().size(), 1);
    EXPECT_EQ(static_cast<uint8_t>(res1.value()[0]), 0x01);

    // Full 8 bits
    auto res8 = data_helper::packCoilBits("10000001", 8);
    ASSERT_TRUE(res8.isOk());
    ASSERT_EQ(res8.value().size(), 1);
    EXPECT_EQ(static_cast<uint8_t>(res8.value()[0]), 0x81);

    // Errors
    EXPECT_TRUE(data_helper::packCoilBits("110100001", 8).isErr()); // Quantity mismatch
    EXPECT_TRUE(data_helper::packCoilBits("10120000", 8).isErr());  // Non-bit character
    EXPECT_TRUE(data_helper::packCoilBits("1", 0).isErr());         // Invalid quantity
}

TEST(ModbusDataHelperTest, ParseRegisterWords) {
    // Decimal parsing
    auto resDec = data_helper::parseRegisterWords("1000 2000", 10, 2);
    ASSERT_TRUE(resDec.isOk());
    ASSERT_EQ(resDec.value().size(), 4);
    // 1000 = 0x03E8
    EXPECT_EQ(static_cast<uint8_t>(resDec.value()[0]), 0x03);
    EXPECT_EQ(static_cast<uint8_t>(resDec.value()[1]), 0xE8);
    // 2000 = 0x07D0
    EXPECT_EQ(static_cast<uint8_t>(resDec.value()[2]), 0x07);
    EXPECT_EQ(static_cast<uint8_t>(resDec.value()[3]), 0xD0);

    // Hex parsing
    auto resHex = data_helper::parseRegisterWords("0x0102, 0x0304", 16, 2);
    ASSERT_TRUE(resHex.isOk());
    ASSERT_EQ(resHex.value().size(), 4);
    EXPECT_EQ(static_cast<uint8_t>(resHex.value()[0]), 0x01);
    EXPECT_EQ(static_cast<uint8_t>(resHex.value()[1]), 0x02);
    EXPECT_EQ(static_cast<uint8_t>(resHex.value()[2]), 0x03);
    EXPECT_EQ(static_cast<uint8_t>(resHex.value()[3]), 0x04);

    // Errors
    EXPECT_TRUE(data_helper::parseRegisterWords("1000", 10, 2).isErr());       // Quantity mismatch
    EXPECT_TRUE(data_helper::parseRegisterWords("70000", 10, 1).isErr());      // 16-bit Overflow
    EXPECT_TRUE(data_helper::parseRegisterWords("invalid", 10, 1).isErr());    // Invalid text
    EXPECT_TRUE(data_helper::parseRegisterWords("100", 8, 1).isErr());         // Unsupported radix
}
