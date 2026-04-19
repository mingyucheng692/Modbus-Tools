#include <gtest/gtest.h>
#include "analyzer/ValueFormatter.h"
#include <QVariant>
#include <limits>

using namespace modbus::analyzer;

TEST(ValueFormatterSafetyTest, InvalidInputs) {
    // Null variant
    EXPECT_EQ(ValueFormatter::formatDecimalValue(QVariant(), NumberDisplayMode::Unsigned), "-");
    
    // Invalid type (String that isn't a number)
    EXPECT_EQ(ValueFormatter::formatDecimalValue(QVariant("not a number"), NumberDisplayMode::Unsigned), "not a number");
    
    // Non-numeric scale
    DataMetadata meta;
    meta.scale = 1.0;
    // Should handle normally (return "-" as defined in formatScaledValue)
    EXPECT_EQ(ValueFormatter::formatScaledValue(QVariant("bad"), meta, NumberDisplayMode::Unsigned), "-");
}

TEST(ValueFormatterSafetyTest, ExtremeValues) {
    DataMetadata meta;
    meta.scale = 1.0;

    // Zero
    EXPECT_EQ(ValueFormatter::numericValueForDisplay(QVariant(0), NumberDisplayMode::Unsigned), 0.0);
    
    // Max values for 16-bit
    EXPECT_EQ(ValueFormatter::formatDecimalValue(QVariant(65535), NumberDisplayMode::Unsigned), "65535");
    EXPECT_EQ(ValueFormatter::formatDecimalValue(QVariant(65535), NumberDisplayMode::Signed), "-1");
    
    // Double accuracy / Large values
    EXPECT_EQ(ValueFormatter::numericValueForDisplay(QVariant(1e15), NumberDisplayMode::Unsigned), 1e15);
}

TEST(ValueFormatterSafetyTest, TooltipSafety) {
    DataMetadata meta;
    meta.description = "Test Description";
    meta.scale = 0.5;
    
    QString tooltip = ValueFormatter::buildDescriptionTooltip(QVariant(10), meta, NumberDisplayMode::Unsigned);
    EXPECT_TRUE(tooltip.contains("Description: Test Description"));
    EXPECT_TRUE(tooltip.contains("Raw: 10"));
    EXPECT_TRUE(tooltip.contains("Scaled: 5"));
}

TEST(ValueFormatterSafetyTest, HexBinaryEdgeCases) {
    // Large hex string (simulating 32-bit or 64-bit)
    QByteArray longBytes = QByteArray::fromHex("12345678");
    EXPECT_EQ(ValueFormatter::formatHexValue(longBytes, ""), "0x12345678");
    
    // Binary grouping for 8 bits (one hex byte)
    QByteArray byteF = QByteArray::fromHex("0F");
    EXPECT_EQ(ValueFormatter::formatBinaryValue(byteF, ""), "0000 1111");
    
    // Fallback should be returned as-is
    EXPECT_EQ(ValueFormatter::formatBinaryValue(QByteArray(), "N/A"), "N/A");
}
