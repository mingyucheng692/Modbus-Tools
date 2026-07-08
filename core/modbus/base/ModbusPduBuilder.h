/**
 * @file ModbusPduBuilder.h
 * @brief Header file for ModbusPduBuilder.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ModbusFrame.h"
#include "result/Result.h"
#include <QString>
#include <variant>
#include <optional>

namespace modbus::base {

using PduBuildResult = ::core::result::Result<Pdu, QString>;

/**
 * @brief Helper class to build Modbus PDUs from raw parameters.
 * Centralizes protocol-specific formatting (endianness, byte counts, bit packing).
 */
class ModbusPduBuilder {
public:
    /**
     * @brief Build a PDU for read operations (0x01-0x04).
     */
    static PduBuildResult buildReadRequest(FunctionCode fc, int startAddress, int quantity);

    /**
     * @brief Build a PDU for Write Single Coil (0x05).
     * @param value true for ON (0xFF00), false for OFF (0x0000).
     */
    static PduBuildResult buildWriteSingleCoil(int startAddress, bool value);

    /**
     * @brief Build a PDU for Write Single Register (0x06).
     */
    static PduBuildResult buildWriteSingleRegister(int startAddress, uint16_t value);

    /**
     * @brief Build a PDU for Write Multiple Coils (0x0F).
     * @param packedData Pre-packed bytes (LSB-first).
     */
    static PduBuildResult buildWriteMultipleCoils(int startAddress, int quantity, const QByteArray& packedData);

    /**
     * @brief Build a PDU for Write Multiple Registers (0x10).
     * @param data Raw byte data (should be multiple of 2).
     */
    static PduBuildResult buildWriteMultipleRegisters(int startAddress, int quantity, const QByteArray& data);
};

} // namespace modbus::base
