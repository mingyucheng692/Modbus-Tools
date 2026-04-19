#include <gtest/gtest.h>
#include "analyzer/ValueFormatter.h"
#include <QVariant>

using namespace modbus::analyzer;

TEST(ValueFormatterTest, FormatDecimal) {
    // Unsigned
    EXPECT_EQ(ValueFormatter::formatDecimalValue(QVariant(123), NumberDisplayMode::Unsigned), "123");
    
    // Signed (Positive)
    EXPECT_EQ(ValueFormatter::formatDecimalValue(QVariant(123), NumberDisplayMode::Signed), "123");
    
    // Signed (Negative/Wraparound)
    // 0xFFFF in 16-bit signed is -1
    EXPECT_EQ(ValueFormatter::formatDecimalValue(QVariant(0xFFFF), NumberDisplayMode::Signed), "-1");
    
    // Boolean
    EXPECT_EQ(ValueFormatter::formatDecimalValue(QVariant(true), NumberDisplayMode::Unsigned), "1");
    EXPECT_EQ(ValueFormatter::formatDecimalValue(QVariant(false), NumberDisplayMode::Unsigned), "0");
}

TEST(ValueFormatterTest, FormatScaled) {
    DataMetadata meta;
    meta.scale = 0.1;
    
    // 100 * 0.1 = 10
    EXPECT_EQ(ValueFormatter::formatScaledValue(QVariant(100), meta, NumberDisplayMode::Unsigned), "10");
    
    // Signed scaling
    // 0xFFFF (-1) * 0.1 = -0.1
    EXPECT_EQ(ValueFormatter::formatScaledValue(QVariant(0xFFFF), meta, NumberDisplayMode::Signed), "-0.1");
}

TEST(ValueFormatterTest, FormatHex) {
    QByteArray bytes = QByteArray::fromHex("01AB");
    EXPECT_EQ(ValueFormatter::formatHexValue(bytes, ""), "0x01AB");
    
    // Empty input should use fallback
    EXPECT_EQ(ValueFormatter::formatHexValue(QByteArray(), "N/A"), "N/A");
}

TEST(ValueFormatterTest, FormatBinary) {
    QByteArray bytes = QByteArray::fromHex("05"); // 0000 0101
    // Note: bitWidth is derived from the hex string length in current implementation
    // "05" -> 2 chars -> 8 bits
    EXPECT_EQ(ValueFormatter::formatBinaryValue(bytes, ""), "0000 0101");
    
    QByteArray bytes2 = QByteArray::fromHex("0102"); // 16 bits
    EXPECT_EQ(ValueFormatter::formatBinaryValue(bytes2, ""), "0000 0001 0000 0010");
}
