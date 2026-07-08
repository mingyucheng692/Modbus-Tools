/**
 * @file ModbusRequestFactory.h
 * @brief PDU-builder factory. Public API is Qt-free; implementation uses QByteArray/QString via ModbusPduBuilder.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "modbus/base/ModbusFrame.h"
#include <cstdint>
#include <optional>
#include <string>
#include <span>

namespace modbus::request {

struct WriteRequestSpec {
    uint8_t functionCode = 0;
    uint16_t startAddress = 0;
    std::span<const uint8_t> rawData;
    uint16_t quantity = 0;
};

struct ReadRequestSpec {
    uint8_t functionCode = 0;
    uint16_t startAddress = 0;
    uint16_t quantity = 0;
};

struct BuildResult {
    std::optional<modbus::base::Pdu> pdu;
    std::string error;

    bool ok() const { return pdu.has_value(); }
    static BuildResult success(modbus::base::Pdu p) { return {std::move(p), {}}; }
    static BuildResult failure(std::string e) { return {std::nullopt, std::move(e)}; }
};

/**
 * @brief Pure PDU-builder factory. Delegates to ModbusPduBuilder for the actual
 * protocol formatting. This class exists so that UI-layer code (e.g.
 * RequestSubmissionService) can delegate PDU construction without mixing
 * protocol logic into format-parsing logic.
 */
class ModbusRequestFactory {
public:
    BuildResult buildReadRequest(const ReadRequestSpec& spec);
    BuildResult buildWriteRequest(const WriteRequestSpec& spec);
};

} // namespace modbus::request