/**
 * @file ModbusByteOrder.h
 * @brief Common byte and word order transformation utilities for Modbus registers.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ModbusTypes.h"
#include <cstdint>
#include <cstring>
#include <span>

namespace modbus::base {

/**
 * @brief Rearrange 2 bytes (1 register) into destination buffer according to RegisterOrder.
 * @param src Source 2-byte buffer.
 * @param dst Destination 2-byte buffer.
 * @param order Register byte/word order.
 */
inline void rearrangeBytes16(const uint8_t* src, uint8_t* dst, RegisterOrder order) noexcept {
    if (order == RegisterOrder::BADC || order == RegisterOrder::DCBA) {
        dst[0] = src[1];
        dst[1] = src[0];
    } else {
        dst[0] = src[0];
        dst[1] = src[1];
    }
}

/**
 * @brief Rearrange 4 bytes (2 registers) into destination buffer according to RegisterOrder.
 * @param src Source 4-byte buffer.
 * @param dst Destination 4-byte buffer.
 * @param order Register byte/word order.
 */
inline void rearrangeBytes32(const uint8_t* src, uint8_t* dst, RegisterOrder order) noexcept {
    switch (order) {
    case RegisterOrder::ABCD:
        dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3];
        break;
    case RegisterOrder::CDAB:
        dst[0] = src[2]; dst[1] = src[3]; dst[2] = src[0]; dst[3] = src[1];
        break;
    case RegisterOrder::BADC:
        dst[0] = src[1]; dst[1] = src[0]; dst[2] = src[3]; dst[3] = src[2];
        break;
    case RegisterOrder::DCBA:
        dst[0] = src[3]; dst[1] = src[2]; dst[2] = src[1]; dst[3] = src[0];
        break;
    }
}

/**
 * @brief Rearrange 8 bytes (4 registers) into destination buffer according to RegisterOrder.
 * @param src Source 8-byte buffer.
 * @param dst Destination 8-byte buffer.
 * @param order Register byte/word order.
 */
inline void rearrangeBytes64(const uint8_t* src, uint8_t* dst, RegisterOrder order) noexcept {
    switch (order) {
    case RegisterOrder::ABCD:
        std::memcpy(dst, src, 8);
        break;
    case RegisterOrder::CDAB: // Word Reverse (R4, R3, R2, R1)
        dst[0] = src[6]; dst[1] = src[7];
        dst[2] = src[4]; dst[3] = src[5];
        dst[4] = src[2]; dst[5] = src[3];
        dst[6] = src[0]; dst[7] = src[1];
        break;
    case RegisterOrder::BADC: // Byte Swap per word
        dst[0] = src[1]; dst[1] = src[0];
        dst[2] = src[3]; dst[3] = src[2];
        dst[4] = src[5]; dst[5] = src[4];
        dst[6] = src[7]; dst[7] = src[6];
        break;
    case RegisterOrder::DCBA: // Pure Little-Endian reverse
        for (int i = 0; i < 8; ++i) {
            dst[i] = src[7 - i];
        }
        break;
    }
}

/**
 * @brief Rearrange 2 bytes (1 register) and return as uint16_t.
 * @param p Pointer to at least 2 raw bytes.
 * @param order Register byte/word order.
 * @return Reordered 16-bit unsigned integer.
 */
[[nodiscard]] inline uint16_t rearrange16(const uint8_t* p, RegisterOrder order) noexcept {
    uint8_t buf[2];
    rearrangeBytes16(p, buf, order);
    return (static_cast<uint16_t>(buf[0]) << 8) | static_cast<uint16_t>(buf[1]);
}

/**
 * @brief Rearrange 4 bytes (2 registers) and return as uint32_t.
 * @param p Pointer to at least 4 raw bytes.
 * @param order Register byte/word order.
 * @return Reordered 32-bit unsigned integer.
 */
[[nodiscard]] inline uint32_t rearrange32(const uint8_t* p, RegisterOrder order) noexcept {
    uint8_t buf[4];
    rearrangeBytes32(p, buf, order);
    return (static_cast<uint32_t>(buf[0]) << 24) |
           (static_cast<uint32_t>(buf[1]) << 16) |
           (static_cast<uint32_t>(buf[2]) << 8)  |
            static_cast<uint32_t>(buf[3]);
}

/**
 * @brief Rearrange 8 bytes (4 registers) and return as uint64_t.
 * @param p Pointer to at least 8 raw bytes.
 * @param order Register byte/word order.
 * @return Reordered 64-bit unsigned integer.
 */
[[nodiscard]] inline uint64_t rearrange64(const uint8_t* p, RegisterOrder order) noexcept {
    uint8_t buf[8];
    rearrangeBytes64(p, buf, order);
    uint64_t u64 = 0;
    for (int i = 0; i < 8; ++i) {
        u64 = (u64 << 8) | static_cast<uint64_t>(buf[i]);
    }
    return u64;
}

// Span-based overloads for strict bounds safety
[[nodiscard]] inline uint16_t rearrange16(std::span<const uint8_t, 2> src, RegisterOrder order) noexcept {
    return rearrange16(src.data(), order);
}

[[nodiscard]] inline uint32_t rearrange32(std::span<const uint8_t, 4> src, RegisterOrder order) noexcept {
    return rearrange32(src.data(), order);
}

[[nodiscard]] inline uint64_t rearrange64(std::span<const uint8_t, 8> src, RegisterOrder order) noexcept {
    return rearrange64(src.data(), order);
}

} // namespace modbus::base
