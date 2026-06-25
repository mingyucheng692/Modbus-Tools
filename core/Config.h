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
    /// Maximum ADU (Application Data Unit) size in bytes.
    /// Per Modbus spec, the maximum PDU is 253 bytes; with 7 bytes MBAP overhead
    /// for TCP (Transaction ID 2 + Protocol ID 2 + Length 2 + Unit ID 1) the max
    /// TCP ADU = 7 + 253 = 260. For RTU, 1 slave + 253 PDU + 2 CRC = 256.
    /// 260 safely covers either transport.
    /// NOTE: For the MBAP Length field limit, use
    /// app::constants::Values::Modbus::kMaxTcpMbapLength (= 254 = Unit ID 1 + PDU 253).
    static constexpr int kMaxAduSize = 260;
};

struct Io {
    /// Maximum chunk size for file transfer via sendFile().
    /// Clamped to prevent memory exhaustion from unreasonable user input.
    static constexpr int kMaxChunkSizeBytes = 65536;
};

} // namespace config