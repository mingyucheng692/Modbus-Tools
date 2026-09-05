/**
 * @file RegisterValueDecoderTest.cpp
 * @brief Unit tests for RegisterValueDecoder, word ordering, and IEEE 754 precision constraints.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "modbus/base/RegisterValueDecoder.h"

using namespace modbus::codec;
using namespace modbus::analyzer;
using namespace modbus::base;

TEST(RegisterValueDecoderTest, UnderflowProtection) {
    const char singleByte[] = { 0x12 };
    auto r1 = decodeRegister(QByteArrayView(singleByte, 1), RegisterDataType::UInt16, RegisterOrder::ABCD);
    EXPECT_FALSE(r1.isValid);
    EXPECT_EQ(r1.displayText, QStringLiteral("<Incomplete>"));

    const char twoBytes[] = { 0x12, 0x34 };
    auto r2 = decodeRegister(QByteArrayView(twoBytes, 2), RegisterDataType::Float32, RegisterOrder::ABCD);
    EXPECT_FALSE(r2.isValid);
    EXPECT_EQ(r2.displayText, QStringLiteral("<Incomplete>"));

    const char fourBytes[] = { 0x12, 0x34, 0x56, 0x78 };
    auto r3 = decodeRegister(QByteArrayView(fourBytes, 4), RegisterDataType::Float64, RegisterOrder::ABCD);
    EXPECT_FALSE(r3.isValid);
    EXPECT_EQ(r3.displayText, QStringLiteral("<Incomplete>"));
}

TEST(RegisterValueDecoderTest, UInt16AndInt16) {
    const char bytes[] = { static_cast<char>(0xFF), static_cast<char>(0xFE) }; // 65534 or -2
    auto uVal = decodeRegister(QByteArrayView(bytes, 2), RegisterDataType::UInt16, RegisterOrder::ABCD);
    EXPECT_TRUE(uVal.isValid);
    EXPECT_EQ(uVal.numericValue, 65534.0);
    EXPECT_EQ(uVal.displayText, QStringLiteral("65534"));

    auto sVal = decodeRegister(QByteArrayView(bytes, 2), RegisterDataType::Int16, RegisterOrder::ABCD);
    EXPECT_TRUE(sVal.isValid);
    EXPECT_EQ(sVal.numericValue, -2.0);
    EXPECT_EQ(sVal.displayText, QStringLiteral("-2"));

    // Byte swap (BADC)
    auto swapped = decodeRegister(QByteArrayView(bytes, 2), RegisterDataType::UInt16, RegisterOrder::BADC);
    EXPECT_TRUE(swapped.isValid);
    EXPECT_EQ(swapped.numericValue, 0xFEFF);
}

TEST(RegisterValueDecoderTest, UInt32WordOrders) {
    const char bytes[] = { 0x12, 0x34, 0x56, 0x78 };

    auto abcd = decodeRegister(QByteArrayView(bytes, 4), RegisterDataType::UInt32, RegisterOrder::ABCD);
    EXPECT_EQ(abcd.numericValue, 0x12345678);

    auto cdab = decodeRegister(QByteArrayView(bytes, 4), RegisterDataType::UInt32, RegisterOrder::CDAB);
    EXPECT_EQ(cdab.numericValue, 0x56781234);

    auto badc = decodeRegister(QByteArrayView(bytes, 4), RegisterDataType::UInt32, RegisterOrder::BADC);
    EXPECT_EQ(badc.numericValue, 0x34127856);

    auto dcba = decodeRegister(QByteArrayView(bytes, 4), RegisterDataType::UInt32, RegisterOrder::DCBA);
    EXPECT_EQ(dcba.numericValue, 0x78563412);
}

TEST(RegisterValueDecoderTest, Float32WordOrders) {
    // 100.0f in IEEE 754 is 0x42C80000:
    // ABCD: 42 C8 00 00
    // CDAB: 00 00 42 C8
    // BADC: C8 42 00 00
    // DCBA: 00 00 C8 42
    const char abcdBytes[] = { 0x42, static_cast<char>(0xC8), 0x00, 0x00 };
    const char cdabBytes[] = { 0x00, 0x00, 0x42, static_cast<char>(0xC8) };
    const char badcBytes[] = { static_cast<char>(0xC8), 0x42, 0x00, 0x00 };
    const char dcbaBytes[] = { 0x00, 0x00, static_cast<char>(0xC8), 0x42 };

    auto r1 = decodeRegister(QByteArrayView(abcdBytes, 4), RegisterDataType::Float32, RegisterOrder::ABCD);
    EXPECT_TRUE(r1.isValid);
    EXPECT_FLOAT_EQ(static_cast<float>(r1.numericValue), 100.0f);
    EXPECT_EQ(r1.displayText, QStringLiteral("100"));

    auto r2 = decodeRegister(QByteArrayView(cdabBytes, 4), RegisterDataType::Float32, RegisterOrder::CDAB);
    EXPECT_TRUE(r2.isValid);
    EXPECT_FLOAT_EQ(static_cast<float>(r2.numericValue), 100.0f);

    auto r3 = decodeRegister(QByteArrayView(badcBytes, 4), RegisterDataType::Float32, RegisterOrder::BADC);
    EXPECT_TRUE(r3.isValid);
    EXPECT_FLOAT_EQ(static_cast<float>(r3.numericValue), 100.0f);

    auto r4 = decodeRegister(QByteArrayView(dcbaBytes, 4), RegisterDataType::Float32, RegisterOrder::DCBA);
    EXPECT_TRUE(r4.isValid);
    EXPECT_FLOAT_EQ(static_cast<float>(r4.numericValue), 100.0f);
}

TEST(RegisterValueDecoderTest, Float32SpecialValues) {
    // NaN: 0x7FC00000
    const char nanBytes[] = { 0x7F, static_cast<char>(0xC0), 0x00, 0x00 };
    auto rNan = decodeRegister(QByteArrayView(nanBytes, 4), RegisterDataType::Float32, RegisterOrder::ABCD);
    EXPECT_FALSE(rNan.isValid);
    EXPECT_TRUE(rNan.isNanOrInf);
    EXPECT_EQ(rNan.displayText, QStringLiteral("NaN"));

    // +Inf: 0x7F800000
    const char infBytes[] = { 0x7F, static_cast<char>(0x80), 0x00, 0x00 };
    auto rInf = decodeRegister(QByteArrayView(infBytes, 4), RegisterDataType::Float32, RegisterOrder::ABCD);
    EXPECT_FALSE(rInf.isValid);
    EXPECT_TRUE(rInf.isNanOrInf);
    EXPECT_EQ(rInf.displayText, QStringLiteral("+Inf"));

    // -Inf: 0xFF800000
    const char negInfBytes[] = { static_cast<char>(0xFF), static_cast<char>(0x80), 0x00, 0x00 };
    auto rNegInf = decodeRegister(QByteArrayView(negInfBytes, 4), RegisterDataType::Float32, RegisterOrder::ABCD);
    EXPECT_FALSE(rNegInf.isValid);
    EXPECT_TRUE(rNegInf.isNanOrInf);
    EXPECT_EQ(rNegInf.displayText, QStringLiteral("-Inf"));

    // -0.0f: 0x80000000 -> Should normalize to "0"
    const char negZeroBytes[] = { static_cast<char>(0x80), 0x00, 0x00, 0x00 };
    auto rNegZero = decodeRegister(QByteArrayView(negZeroBytes, 4), RegisterDataType::Float32, RegisterOrder::ABCD);
    EXPECT_TRUE(rNegZero.isValid);
    EXPECT_FALSE(rNegZero.isNanOrInf);
    EXPECT_EQ(rNegZero.displayText, QStringLiteral("0"));
}

TEST(RegisterValueDecoderTest, Float64Decoding) {
    // 1.0 (double) in IEEE 754 is 0x3FF0000000000000ULL:
    // ABCD: 3F F0 00 00 00 00 00 00
    // CDAB (Word Reverse R4..R1): 00 00 00 00 00 00 3F F0
    const char abcdDouble[] = { 0x3F, static_cast<char>(0xF0), 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    auto r1 = decodeRegister(QByteArrayView(abcdDouble, 8), RegisterDataType::Float64, RegisterOrder::ABCD);
    EXPECT_TRUE(r1.isValid);
    EXPECT_DOUBLE_EQ(r1.numericValue, 1.0);
    EXPECT_EQ(r1.displayText, QStringLiteral("1"));

    const char cdabDouble[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, static_cast<char>(0xF0) };
    auto r2 = decodeRegister(QByteArrayView(cdabDouble, 8), RegisterDataType::Float64, RegisterOrder::CDAB);
    EXPECT_TRUE(r2.isValid);
    EXPECT_DOUBLE_EQ(r2.numericValue, 1.0);
}

TEST(RegisterValueDecoderTest, ScientificNotationAndPrecision) {
    // Zero
    EXPECT_EQ(formatFloatScientific(0.0, 7), QStringLiteral("0"));
    EXPECT_EQ(formatFloatScientific(-0.0, 7), QStringLiteral("0"));

    // Small numbers (< 1e-4) -> Forced scientific notation with stripped zeros
    EXPECT_EQ(formatFloatScientific(1.234567e-5, 7), QStringLiteral("1.234567e-05"));
    EXPECT_EQ(formatFloatScientific(1.2e-5, 7), QStringLiteral("1.2e-05"));
    EXPECT_EQ(formatFloatScientific(1e-5, 7), QStringLiteral("1e-05"));

    // Normal range -> Compact decimal without trailing zeros
    EXPECT_EQ(formatFloatScientific(123.45, 7), QStringLiteral("123.45"));
    EXPECT_EQ(formatFloatScientific(0.001234567, 7), QStringLiteral("0.001234567"));

    // Large numbers (>= 1e7 for 7 sig digits) -> Forced scientific notation
    EXPECT_EQ(formatFloatScientific(12345670.0, 7), QStringLiteral("1.234567e+07"));
}

TEST(RegisterValueDecoderTest, FormatEngineeringValueWithScale) {
    DecodedRegisterValue normal;
    normal.isValid = true;
    normal.numericValue = 100.0;
    normal.displayText = QStringLiteral("100");

    EXPECT_EQ(formatEngineeringValue(normal, 1.0, RegisterDataType::Float32), QStringLiteral("100"));
    EXPECT_EQ(formatEngineeringValue(normal, 0.1, RegisterDataType::Float32), QStringLiteral("10"));

    // NaN / Inf protection: do NOT multiply, prevent NaN pollution
    DecodedRegisterValue nanVal;
    nanVal.isValid = false;
    nanVal.isNanOrInf = true;
    nanVal.displayText = QStringLiteral("NaN");

    EXPECT_EQ(formatEngineeringValue(nanVal, 2.5, RegisterDataType::Float32), QStringLiteral("NaN"));

    DecodedRegisterValue infVal;
    infVal.isValid = false;
    infVal.isNanOrInf = true;
    infVal.displayText = QStringLiteral("+Inf");

    EXPECT_EQ(formatEngineeringValue(infVal, 0.5, RegisterDataType::Float32), QStringLiteral("+Inf"));
}
