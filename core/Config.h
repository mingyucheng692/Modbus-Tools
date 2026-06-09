/**
 * @file Config.h
 * @brief Application-wide typed configuration constants.
 * 
 * Centralizes magic numbers into `config::` namespace per AGENTS.md config discipline.
 * Existing `app::constants::Values` constants remain for backward compatibility;
 * new hard limits and structural constants live here.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <cstddef>

namespace config {

struct Modbus {
    /// Maximum ADU (Application Data Unit) size in bytes (256 data + 4 overhead).
    /// Per Modbus spec, the maximum PDU is 253 bytes; with 7 bytes overhead
    /// (1 slave + 2 CRC for RTU, or 7 MBAP for TCP) this safely covers either.
    static constexpr int kMaxAdpuSize = 260;

    /// Maximum MBAP Length field value per Modbus TCP spec.
    /// Unit ID (1) + Function Code (1) + Data (253) = 255 max.
    static constexpr int kMaxMbapLength = 255;
};

struct Io {
    /// Maximum chunk size for file transfer via sendFile().
    /// Clamped to prevent memory exhaustion from unreasonable user input.
    static constexpr int kMaxChunkSizeBytes = 65536;
};

} // namespace config