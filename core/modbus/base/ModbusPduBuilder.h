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
 * @brief Helper namespace to build Modbus PDUs from raw parameters.
 * Centralizes protocol-specific formatting (endianness, byte counts, bit packing).
 *
 * All build functions return std::optional<Pdu>. On failure, returns
 * std::nullopt and (if errorOut is non-null) writes a human-readable
 * description to *errorOut. This replaces the former Result<T,E> pattern.
 */
namespace pdu_builder {

/**
 * @brief Build a PDU for read operations (0x01-0x04).
 */
std::optional<Pdu> buildReadRequest(FunctionCode fc, int startAddress, int quantity,
                                    QString* errorOut = nullptr);

/**
 * @brief Build a PDU for write operations (0x05/0x06/0x0F/0x10).
 * Dispatches to the specific write function based on function code.
 * @param rawData For 0x05/0x06: 1-2 bytes interpreted as bool/uint16_t
 *                (big-endian; 0xFF00 = ON for 0x05).
 *                For 0x0F/0x10: packed byte data.
 */
std::optional<Pdu> buildWriteRequest(FunctionCode fc, int startAddress, int quantity,
                                     const QByteArray& rawData, QString* errorOut = nullptr);

/**
 * @brief Build a PDU for Write Single Coil (0x05).
 * @param value true for ON (0xFF00), false for OFF (0x0000).
 */
std::optional<Pdu> buildWriteSingleCoil(int startAddress, bool value,
                                         QString* errorOut = nullptr);

/**
 * @brief Build a PDU for Write Single Register (0x06).
 */
std::optional<Pdu> buildWriteSingleRegister(int startAddress, uint16_t value,
                                            QString* errorOut = nullptr);

/**
 * @brief Build a PDU for Write Multiple Coils (0x0F).
 * @param packedData Pre-packed bytes (LSB-first).
 */
std::optional<Pdu> buildWriteMultipleCoils(int startAddress, int quantity,
                                            const QByteArray& packedData,
                                            QString* errorOut = nullptr);

/**
 * @brief Build a PDU for Write Multiple Registers (0x10).
 * @param data Raw byte data (should be multiple of 2).
 */
std::optional<Pdu> buildWriteMultipleRegisters(int startAddress, int quantity,
                                               const QByteArray& data,
                                               QString* errorOut = nullptr);

} // namespace pdu_builder

} // namespace modbus::base
