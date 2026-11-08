// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "analyzer/ValueFormatter.h"
#include <QVariant>
#include <limits>

using namespace modbus::analyzer;

TEST(ValueFormatterSafetyTest, InvalidInputs) {
    // Null variant
    EXPECT_EQ(value_formatter::formatDecimalValue(QVariant(), NumberDisplayMode::Unsigned), "-");
    
    // Invalid type (String that isn't a number)
    EXPECT_EQ(value_formatter::formatDecimalValue(QVariant("not a number"), NumberDisplayMode::Unsigned), "not a number");
    
    // Non-numeric scale
    DataMetadata meta;
    meta.scale = 1.0;
    // Should handle normally (return "-" as defined in formatScaledValue)
    EXPECT_EQ(value_formatter::formatScaledValue(QVariant("bad"), meta, NumberDisplayMode::Unsigned), "-");
}

TEST(ValueFormatterSafetyTest, ExtremeValues) {
    DataMetadata meta;
    meta.scale = 1.0;

    // Zero
    EXPECT_EQ(value_formatter::numericValueForDisplay(QVariant(0), NumberDisplayMode::Unsigned), 0.0);
    
    // Max values for 16-bit
    EXPECT_EQ(value_formatter::formatDecimalValue(QVariant(65535), NumberDisplayMode::Unsigned), "65535");
    EXPECT_EQ(value_formatter::formatDecimalValue(QVariant(65535), NumberDisplayMode::Signed), "-1");
    
    // Double accuracy / Large values
    EXPECT_EQ(value_formatter::numericValueForDisplay(QVariant(1e15), NumberDisplayMode::Unsigned), 1e15);
}

TEST(ValueFormatterSafetyTest, TooltipSafety) {
    DataMetadata meta;
    meta.description = "Test Description";
    meta.scale = 0.5;
    
    QString tooltip = value_formatter::buildDescriptionTooltip(QVariant(10), meta, NumberDisplayMode::Unsigned);
    EXPECT_TRUE(tooltip.contains("Description: Test Description"));
    EXPECT_TRUE(tooltip.contains("Raw: 10"));
    EXPECT_TRUE(tooltip.contains("Scaled: 5"));
}

TEST(ValueFormatterSafetyTest, HexBinaryEdgeCases) {
    // Large hex string (simulating 32-bit or 64-bit)
    QByteArray longBytes = QByteArray::fromHex("12345678");
    EXPECT_EQ(value_formatter::formatHexValue(longBytes, ""), "0x12345678");

    // Binary grouping for 8 bits (one hex byte)
    QByteArray byteF = QByteArray::fromHex("0F");
    EXPECT_EQ(value_formatter::formatBinaryValue(byteF, ""), "0000 1111");

    // Fallback should be returned as-is
    EXPECT_EQ(value_formatter::formatBinaryValue(QByteArray(), "N/A"), "N/A");
}

TEST(ValueFormatterSafetyTest, QVariantNaNInfIntercepted) {
    DataMetadata meta;
    meta.scale = 2.0;

    // A QVariant double can carry NaN/±Inf (e.g. an IEEE-754 payload decoded
    // as float); the scaled output must never render as "nan"/"inf" — the
    // QVariant path must match the rawBytes path's decoding guard.
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double inf = std::numeric_limits<double>::infinity();
    EXPECT_EQ(value_formatter::formatScaledValue(QVariant(nan), meta, NumberDisplayMode::Unsigned), "-");
    EXPECT_EQ(value_formatter::formatScaledValue(QVariant(inf), meta, NumberDisplayMode::Unsigned), "-");
    EXPECT_EQ(value_formatter::formatScaledValue(QVariant(-inf), meta, NumberDisplayMode::Unsigned), "-");

    // Tooltip path must not emit a "Scaled: nan" line either.
    const QString tooltip = value_formatter::buildDescriptionTooltip(QVariant(nan), meta, NumberDisplayMode::Unsigned);
    EXPECT_FALSE(tooltip.contains("nan"));
    EXPECT_FALSE(tooltip.contains("inf"));
    EXPECT_FALSE(tooltip.contains("Scaled:"));
}
