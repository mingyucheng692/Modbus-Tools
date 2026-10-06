/**
 * @file ModbusLrc.h
 * @brief Modbus ASCII LRC (Longitudinal Redundancy Check) calculation.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <cstdint>

namespace modbus::base {

/// Calculates the Modbus ASCII LRC checksum.  The LRC is the two's-complement
/// negation of the sum of all bytes in @p data.  Inline (header-only) because
/// the implementation is 7 lines and mirrors ModbusCrc's header-only style.
[[nodiscard]] inline uint8_t calculateModbusAsciiLrc(QByteArrayView data) noexcept {
    uint8_t sum = 0;
    for (const char byte : data) {
        sum = static_cast<uint8_t>(sum + static_cast<uint8_t>(byte));
    }
    return static_cast<uint8_t>(-sum);
}

/**
 * @brief Calculate and append Modbus ASCII LRC byte to @p buffer.
 * @param buffer Output buffer to append 1-byte LRC to.
 */
inline void appendModbusAsciiLrc(QByteArray& buffer) noexcept {
    const uint8_t lrc = calculateModbusAsciiLrc(buffer);
    buffer.append(static_cast<char>(lrc));
}

/**
 * @brief Verify that the trailing byte of @p buffer matches its calculated Modbus ASCII LRC.
 * @param buffer Buffer containing binary payload + 1-byte LRC.
 * @return true if valid, false if buffer is too short or LRC does not match.
 */
[[nodiscard]] inline bool verifyModbusAsciiLrc(QByteArrayView buffer) noexcept {
    if (buffer.size() < 2) {
        return false;
    }
    const auto receivedLrc = static_cast<uint8_t>(buffer.back());
    const uint8_t calculatedLrc = calculateModbusAsciiLrc(buffer.first(buffer.size() - 1));
    return receivedLrc == calculatedLrc;
}

} // namespace modbus::base
