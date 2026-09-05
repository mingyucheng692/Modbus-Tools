/**
 * @file ModbusAddressMappingTest.cpp
 * @brief Unit tests for ModbusAddressMapping logic.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>
#include "modbus/base/ModbusAddressMapping.h"

using namespace modbus::address;

TEST(ModbusAddressMappingTest, Offset0Based_ValidInputs) {
    auto r1 = toPduAddress(QStringLiteral("0"), AddressBase::Offset0Based);
    EXPECT_TRUE(r1.isValid);
    EXPECT_EQ(r1.pduAddress, 0);

    auto r2 = toPduAddress(QStringLiteral("65535"), AddressBase::Offset0Based);
    EXPECT_TRUE(r2.isValid);
    EXPECT_EQ(r2.pduAddress, 65535);

    auto r3 = toPduAddress(QStringLiteral("0x0010"), AddressBase::Offset0Based);
    EXPECT_TRUE(r3.isValid);
    EXPECT_EQ(r3.pduAddress, 16);

    auto r4 = toPduAddress(QStringLiteral("10H"), AddressBase::Offset0Based);
    EXPECT_TRUE(r4.isValid);
    EXPECT_EQ(r4.pduAddress, 16);

    auto r5 = toPduAddress(QStringLiteral("  100  "), AddressBase::Offset0Based);
    EXPECT_TRUE(r5.isValid);
    EXPECT_EQ(r5.pduAddress, 100);
}

TEST(ModbusAddressMappingTest, Offset0Based_InvalidInputs) {
    auto r1 = toPduAddress(QStringLiteral(""), AddressBase::Offset0Based);
    EXPECT_FALSE(r1.isValid);

    auto r2 = toPduAddress(QStringLiteral("-1"), AddressBase::Offset0Based);
    EXPECT_FALSE(r2.isValid);

    auto r3 = toPduAddress(QStringLiteral("65536"), AddressBase::Offset0Based);
    EXPECT_FALSE(r3.isValid);

    auto r4 = toPduAddress(QStringLiteral("0x10H"), AddressBase::Offset0Based);
    EXPECT_FALSE(r4.isValid);

    auto r5 = toPduAddress(QStringLiteral("invalid"), AddressBase::Offset0Based);
    EXPECT_FALSE(r5.isValid);
}

TEST(ModbusAddressMappingTest, PlcAddress1Based_PlainNumbers) {
    auto r1 = toPduAddress(QStringLiteral("1"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r1.isValid);
    EXPECT_EQ(r1.pduAddress, 0);

    auto r2 = toPduAddress(QStringLiteral("65536"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r2.isValid);
    EXPECT_EQ(r2.pduAddress, 65535);

    // 0 is strictly invalid in 1-based mode
    auto r3 = toPduAddress(QStringLiteral("0"), AddressBase::PlcAddress1Based);
    EXPECT_FALSE(r3.isValid);
    EXPECT_TRUE(r3.errorMessage.contains(QStringLiteral(">= 1")));

    auto r4 = toPduAddress(QStringLiteral("65537"), AddressBase::PlcAddress1Based);
    EXPECT_FALSE(r4.isValid);

    // Hex in 1-based mode
    auto r5 = toPduAddress(QStringLiteral("0x0001"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r5.isValid);
    EXPECT_EQ(r5.pduAddress, 0);

    auto r6 = toPduAddress(QStringLiteral("0x0000"), AddressBase::PlcAddress1Based);
    EXPECT_FALSE(r6.isValid);
}

TEST(ModbusAddressMappingTest, PlcAddress1Based_Modicon5DigitPrefixes) {
    // 40001 -> Holding Register FC03, PDU 0
    auto r1 = toPduAddress(QStringLiteral("40001"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r1.isValid);
    EXPECT_EQ(r1.pduAddress, 0);
    EXPECT_EQ(r1.suggestedFunctionCode, std::optional<uint8_t>(0x03));

    auto r2 = toPduAddress(QStringLiteral("40100"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r2.isValid);
    EXPECT_EQ(r2.pduAddress, 99);
    EXPECT_EQ(r2.suggestedFunctionCode, std::optional<uint8_t>(0x03));

    // 30001 -> Input Register FC04, PDU 0
    auto r3 = toPduAddress(QStringLiteral("30001"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r3.isValid);
    EXPECT_EQ(r3.pduAddress, 0);
    EXPECT_EQ(r3.suggestedFunctionCode, std::optional<uint8_t>(0x04));

    // 10001 -> Discrete Input FC02, PDU 0
    auto r4 = toPduAddress(QStringLiteral("10001"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r4.isValid);
    EXPECT_EQ(r4.pduAddress, 0);
    EXPECT_EQ(r4.suggestedFunctionCode, std::optional<uint8_t>(0x02));

    // 00001 -> Coil FC01, PDU 0
    auto r5 = toPduAddress(QStringLiteral("00001"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r5.isValid);
    EXPECT_EQ(r5.pduAddress, 0);
    EXPECT_EQ(r5.suggestedFunctionCode, std::optional<uint8_t>(0x01));
}

TEST(ModbusAddressMappingTest, PlcAddress1Based_Modicon6DigitPrefixes) {
    // 400001 -> Holding Register FC03, PDU 0
    auto r1 = toPduAddress(QStringLiteral("400001"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r1.isValid);
    EXPECT_EQ(r1.pduAddress, 0);
    EXPECT_EQ(r1.suggestedFunctionCode, std::optional<uint8_t>(0x03));

    // 465536 -> Holding Register FC03, PDU 65535
    auto r2 = toPduAddress(QStringLiteral("465536"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r2.isValid);
    EXPECT_EQ(r2.pduAddress, 65535);
    EXPECT_EQ(r2.suggestedFunctionCode, std::optional<uint8_t>(0x03));

    // 300001 -> Input Register FC04, PDU 0
    auto r3 = toPduAddress(QStringLiteral("300001"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r3.isValid);
    EXPECT_EQ(r3.pduAddress, 0);
    EXPECT_EQ(r3.suggestedFunctionCode, std::optional<uint8_t>(0x04));

    // 100001 -> Discrete Input FC02, PDU 0
    auto r4 = toPduAddress(QStringLiteral("100001"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r4.isValid);
    EXPECT_EQ(r4.pduAddress, 0);
    EXPECT_EQ(r4.suggestedFunctionCode, std::optional<uint8_t>(0x02));

    // 000001 -> Coil FC01, PDU 0
    auto r5 = toPduAddress(QStringLiteral("000001"), AddressBase::PlcAddress1Based);
    EXPECT_TRUE(r5.isValid);
    EXPECT_EQ(r5.pduAddress, 0);
    EXPECT_EQ(r5.suggestedFunctionCode, std::optional<uint8_t>(0x01));
}

TEST(ModbusAddressMappingTest, ToDisplayAddress) {
    EXPECT_EQ(toDisplayAddress(0, AddressBase::Offset0Based, false), QStringLiteral("0"));
    EXPECT_EQ(toDisplayAddress(0, AddressBase::Offset0Based, true), QStringLiteral("0x0000"));
    EXPECT_EQ(toDisplayAddress(15, AddressBase::Offset0Based, true), QStringLiteral("0x000F"));

    EXPECT_EQ(toDisplayAddress(0, AddressBase::PlcAddress1Based, false), QStringLiteral("1"));
    EXPECT_EQ(toDisplayAddress(0, AddressBase::PlcAddress1Based, true), QStringLiteral("0x0001"));
    EXPECT_EQ(toDisplayAddress(65535, AddressBase::PlcAddress1Based, false), QStringLiteral("65536"));
}
