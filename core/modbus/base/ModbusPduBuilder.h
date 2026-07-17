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
#include <QString>
#include <variant>
#include <optional>

namespace modbus::base {

/**
 * @brief Helper class to build Modbus PDUs from raw parameters.
 * Centralizes protocol-specific formatting (endianness, byte counts, bit packing).
 *
 * All build methods return std::optional<Pdu>. On failure, returns
 * std::nullopt and (if errorOut is non-null) writes a human-readable
 * description to *errorOut. This replaces the former Result<T,E> pattern.
 */
class ModbusPduBuilder {
public:
    /**
     * @brief Build a PDU for read operations (0x01-0x04).
     */
    static std::optional<Pdu> buildReadRequest(FunctionCode fc, int startAddress, int quantity,
                                                QString* errorOut = nullptr);

    /**
     * @brief Build a PDU for Write Single Coil (0x05).
     * @param value true for ON (0xFF00), false for OFF (0x0000).
     */
    static std::optional<Pdu> buildWriteSingleCoil(int startAddress, bool value,
                                                     QString* errorOut = nullptr);

    /**
     * @brief Build a PDU for Write Single Register (0x06).
     */
    static std::optional<Pdu> buildWriteSingleRegister(int startAddress, uint16_t value,
                                                        QString* errorOut = nullptr);

    /**
     * @brief Build a PDU for Write Multiple Coils (0x0F).
     * @param packedData Pre-packed bytes (LSB-first).
     */
    static std::optional<Pdu> buildWriteMultipleCoils(int startAddress, int quantity,
                                                        const QByteArray& packedData,
                                                        QString* errorOut = nullptr);

    /**
     * @brief Build a PDU for Write Multiple Registers (0x10).
     * @param data Raw byte data (should be multiple of 2).
     */
    static std::optional<Pdu> buildWriteMultipleRegisters(int startAddress, int quantity,
                                                           const QByteArray& data,
                                                           QString* errorOut = nullptr);
};

} // namespace modbus::base
