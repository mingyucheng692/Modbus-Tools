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

/**
 * @brief Pure PDU-builder factory. Delegates to ModbusPduBuilder for the actual
 * protocol formatting. This class exists so that UI-layer code (e.g.
 * RequestSubmissionService) can delegate PDU construction without mixing
 * protocol logic into format-parsing logic.
 *
 * Returns std::optional<Pdu>; on failure, returns std::nullopt and (if
 * errorOut is non-null) writes a description to *errorOut.
 */
class ModbusRequestFactory {
public:
    std::optional<modbus::base::Pdu> buildReadRequest(const ReadRequestSpec& spec,
                                                       std::string* errorOut = nullptr);
    std::optional<modbus::base::Pdu> buildWriteRequest(const WriteRequestSpec& spec,
                                                        std::string* errorOut = nullptr);
};

} // namespace modbus::request