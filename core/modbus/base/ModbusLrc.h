// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <cstdint>

namespace modbus::base {

// Calculates the Modbus ASCII LRC checksum: two's-complement negation of the byte sum.
[[nodiscard]] inline uint8_t calculateModbusAsciiLrc(QByteArrayView data) noexcept {
    uint8_t sum = 0;
    for (const char byte : data) {
        sum = static_cast<uint8_t>(sum + static_cast<uint8_t>(byte));
    }
    return static_cast<uint8_t>(-sum);
}

inline void appendModbusAsciiLrc(QByteArray& buffer) noexcept {
    const uint8_t lrc = calculateModbusAsciiLrc(buffer);
    buffer.append(static_cast<char>(lrc));
}

// Verifies trailing byte matches calculated LRC; returns false if buffer size < 2 or LRC mismatches.
[[nodiscard]] inline bool verifyModbusAsciiLrc(QByteArrayView buffer) noexcept {
    if (buffer.size() < 2) {
        return false;
    }
    const auto receivedLrc = static_cast<uint8_t>(buffer.back());
    const uint8_t calculatedLrc = calculateModbusAsciiLrc(buffer.first(buffer.size() - 1));
    return receivedLrc == calculatedLrc;
}

} // namespace modbus::base
