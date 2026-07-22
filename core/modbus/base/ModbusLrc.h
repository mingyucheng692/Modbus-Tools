/**
 * @file ModbusLrc.h
 * @brief Modbus ASCII LRC (Longitudinal Redundancy Check) calculation.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

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

} // namespace modbus::base
