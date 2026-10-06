/**
 * @file ModbusByteOrderTest.cpp
 * @brief Unit tests for ModbusByteOrder utilities.
 */

#include <gtest/gtest.h>
#include "modbus/base/ModbusByteOrder.h"

using namespace modbus::base;

TEST(ModbusByteOrderTest, Rearrange16_AllOrders) {
    const uint8_t raw[2] = {0x12, 0x34};

    // ABCD / CDAB -> High byte first: 0x1234
    EXPECT_EQ(rearrange16(raw, RegisterOrder::ABCD), 0x1234);
    EXPECT_EQ(rearrange16(raw, RegisterOrder::CDAB), 0x1234);

    // BADC / DCBA -> Low byte first (byte swapped): 0x3412
    EXPECT_EQ(rearrange16(raw, RegisterOrder::BADC), 0x3412);
    EXPECT_EQ(rearrange16(raw, RegisterOrder::DCBA), 0x3412);
}

TEST(ModbusByteOrderTest, Rearrange32_AllOrders) {
    const uint8_t raw[4] = {0x12, 0x34, 0x56, 0x78};

    // ABCD -> 0x12345678 (Big Endian)
    EXPECT_EQ(rearrange32(raw, RegisterOrder::ABCD), 0x12345678);

    // CDAB -> 0x56781234 (Word Swap)
    EXPECT_EQ(rearrange32(raw, RegisterOrder::CDAB), 0x56781234);

    // BADC -> 0x34127856 (Byte Swap per word)
    EXPECT_EQ(rearrange32(raw, RegisterOrder::BADC), 0x34127856);

    // DCBA -> 0x78563412 (Little Endian)
    EXPECT_EQ(rearrange32(raw, RegisterOrder::DCBA), 0x78563412);
}

TEST(ModbusByteOrderTest, Rearrange64_AllOrders) {
    const uint8_t raw[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

    // ABCD -> 0x0102030405060708
    EXPECT_EQ(rearrange64(raw, RegisterOrder::ABCD), 0x0102030405060708ULL);

    // CDAB -> Word Reverse (R4, R3, R2, R1): 0x0708050603040102
    EXPECT_EQ(rearrange64(raw, RegisterOrder::CDAB), 0x0708050603040102ULL);

    // BADC -> Byte Swap per word: 0x0201040306050807
    EXPECT_EQ(rearrange64(raw, RegisterOrder::BADC), 0x0201040306050807ULL);

    // DCBA -> Pure Little Endian reverse: 0x0807060504030201
    EXPECT_EQ(rearrange64(raw, RegisterOrder::DCBA), 0x0807060504030201ULL);
}

TEST(ModbusByteOrderTest, RearrangeBytes_MatchesScalars) {
    const uint8_t raw32[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    for (auto order : {RegisterOrder::ABCD, RegisterOrder::CDAB, RegisterOrder::BADC, RegisterOrder::DCBA}) {
        uint8_t dst[4];
        rearrangeBytes32(raw32, dst, order);
        const uint32_t valFromBytes = (static_cast<uint32_t>(dst[0]) << 24) |
                                      (static_cast<uint32_t>(dst[1]) << 16) |
                                      (static_cast<uint32_t>(dst[2]) << 8)  |
                                       static_cast<uint32_t>(dst[3]);
        EXPECT_EQ(rearrange32(raw32, order), valFromBytes);
    }
}
