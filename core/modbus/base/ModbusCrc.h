// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArray>
#include <QByteArrayView>
#include <cstdint>

namespace modbus::base {

[[nodiscard]] inline uint16_t calculateModbusRtuCrc(QByteArrayView data) noexcept {
    uint16_t crc = 0xFFFF;
    for (char c : data) {
        crc ^= static_cast<uint8_t>(c);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Appends Modbus RTU CRC-16 (little-endian: low byte first) to buffer.
inline void appendModbusRtuCrc(QByteArray& buffer) noexcept {
    const uint16_t crc = calculateModbusRtuCrc(buffer);
    buffer.append(static_cast<char>(crc & 0xFF));
    buffer.append(static_cast<char>((crc >> 8) & 0xFF));
}

// Verifies that trailing 2 bytes of buffer match calculated Modbus RTU CRC-16.
[[nodiscard]] inline bool verifyModbusRtuCrc(QByteArrayView buffer) noexcept {
    if (buffer.size() < 3) {
        return false;
    }
    const auto receivedCrc = static_cast<uint16_t>(
        static_cast<uint8_t>(buffer[buffer.size() - 2]) |
        (static_cast<uint16_t>(static_cast<uint8_t>(buffer[buffer.size() - 1])) << 8));
    const uint16_t calculatedCrc = calculateModbusRtuCrc(buffer.first(buffer.size() - 2));
    return receivedCrc == calculatedCrc;
}

} // namespace modbus::base

