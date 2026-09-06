#include <gtest/gtest.h>
#include "analyzer/ValueFormatter.h"
#include <QVariant>

using namespace modbus::analyzer;

TEST(ValueFormatterTest, FormatDecimal) {
    // Unsigned
    EXPECT_EQ(value_formatter::formatDecimalValue(QVariant(123), NumberDisplayMode::Unsigned), "123");
    
    // Signed (Positive)
    EXPECT_EQ(value_formatter::formatDecimalValue(QVariant(123), NumberDisplayMode::Signed), "123");
    
    // Signed (Negative/Wraparound)
    // 0xFFFF in 16-bit signed is -1
    EXPECT_EQ(value_formatter::formatDecimalValue(QVariant(0xFFFF), NumberDisplayMode::Signed), "-1");
    
    // Boolean
    EXPECT_EQ(value_formatter::formatDecimalValue(QVariant(true), NumberDisplayMode::Unsigned), "1");
    EXPECT_EQ(value_formatter::formatDecimalValue(QVariant(false), NumberDisplayMode::Unsigned), "0");
}

TEST(ValueFormatterTest, FormatScaled) {
    DataMetadata meta;
    meta.scale = 0.1;
    
    // 100 * 0.1 = 10
    EXPECT_EQ(value_formatter::formatScaledValue(QVariant(100), meta, NumberDisplayMode::Unsigned), "10");
    
    // Signed scaling
    // 0xFFFF (-1) * 0.1 = -0.1
    EXPECT_EQ(value_formatter::formatScaledValue(QVariant(0xFFFF), meta, NumberDisplayMode::Signed), "-0.1");
}

TEST(ValueFormatterTest, FormatHex) {
    QByteArray bytes = QByteArray::fromHex("01AB");
    EXPECT_EQ(value_formatter::formatHexValue(bytes, ""), "0x01AB");
    
    // Empty input should use fallback
    EXPECT_EQ(value_formatter::formatHexValue(QByteArray(), "N/A"), "N/A");
}

TEST(ValueFormatterTest, FormatBinary) {
    QByteArray bytes = QByteArray::fromHex("05"); // 0000 0101
    // Note: bitWidth is derived from the hex string length in current implementation
    // "05" -> 2 chars -> 8 bits
    EXPECT_EQ(value_formatter::formatBinaryValue(bytes, ""), "0000 0101");
    
    QByteArray bytes2 = QByteArray::fromHex("0102"); // 16 bits
    EXPECT_EQ(value_formatter::formatBinaryValue(bytes2, ""), "0000 0001 0000 0010");
}

TEST(ValueFormatterTest, FormatScaledMultiRegister) {
    DataMetadata meta;
    meta.scale = 0.1;

    // 32-bit float: 12.5f in big-endian ABCD is 41 48 00 00
    QByteArray floatBytesABCD = QByteArray::fromHex("41480000");
    EXPECT_EQ(value_formatter::formatScaledValue(floatBytesABCD, meta, RegisterDataType::Float32, modbus::base::RegisterOrder::ABCD), "1.25");

    // 32-bit float in CDAB (word swap) is 00 00 41 48
    QByteArray floatBytesCDAB = QByteArray::fromHex("00004148");
    meta.scale = 1.0;
    EXPECT_EQ(value_formatter::formatScaledValue(floatBytesCDAB, meta, RegisterDataType::Float32, modbus::base::RegisterOrder::CDAB), "12.5");

    // Incomplete buffer (only 2 bytes for Float32)
    QByteArray incompleteBytes = QByteArray::fromHex("4148");
    EXPECT_EQ(value_formatter::formatScaledValue(incompleteBytes, meta, RegisterDataType::Float32, modbus::base::RegisterOrder::ABCD), "<Incomplete>");

    // 64-bit float: 12.5 in IEEE 754 double is 40 29 00 00 00 00 00 00
    QByteArray doubleBytes = QByteArray::fromHex("4029000000000000");
    meta.scale = 2.0;
    EXPECT_EQ(value_formatter::formatScaledValue(doubleBytes, meta, RegisterDataType::Float64, modbus::base::RegisterOrder::ABCD), "25");

    // 32-bit integer: 65536 is 00 01 00 00
    QByteArray int32Bytes = QByteArray::fromHex("00010000");
    meta.scale = 1.0;
    EXPECT_EQ(value_formatter::formatScaledValue(int32Bytes, meta, RegisterDataType::Int32, modbus::base::RegisterOrder::ABCD), "65536");
}

TEST(ValueFormatterTest, MultiRegisterTooltip) {
    DataMetadata meta;
    meta.description = "Voltage Sensor";
    meta.scale = 0.5;

    QByteArray floatBytes = QByteArray::fromHex("41480000"); // 12.5
    QString tooltip = value_formatter::buildDescriptionTooltip(floatBytes, meta, RegisterDataType::Float32, modbus::base::RegisterOrder::ABCD);
    EXPECT_TRUE(tooltip.contains("Description: Voltage Sensor"));
    EXPECT_TRUE(tooltip.contains("Raw: 12.5"));
    EXPECT_TRUE(tooltip.contains("Scale: 0.5"));
    EXPECT_TRUE(tooltip.contains("Scaled: 6.25"));
}

