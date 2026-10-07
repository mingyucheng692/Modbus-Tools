// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "ModbusFrame.h"
#include <QString>
#include <variant>
#include <optional>

namespace modbus::base {

// Builds Modbus PDUs from raw parameters; centralizes endianness, byte counts, and bit packing.
// All functions return std::nullopt on failure and optionally write description to errorOut.
namespace pdu_builder {

std::optional<Pdu> buildReadRequest(FunctionCode fc, int startAddress, int quantity,
                                    QString* errorOut = nullptr);

// rawData: Interpreted as bool/uint16_t for 0x05/0x06, or packed byte data for 0x0F/0x10.
std::optional<Pdu> buildWriteRequest(FunctionCode fc, int startAddress, int quantity,
                                     const QByteArray& rawData, QString* errorOut = nullptr);

std::optional<Pdu> buildWriteSingleCoil(int startAddress, bool value,
                                         QString* errorOut = nullptr);

std::optional<Pdu> buildWriteSingleRegister(int startAddress, uint16_t value,
                                            QString* errorOut = nullptr);

// packedData: Pre-packed bytes (LSB-first).
std::optional<Pdu> buildWriteMultipleCoils(int startAddress, int quantity,
                                            const QByteArray& packedData,
                                            QString* errorOut = nullptr);

// data: Raw byte data (should be multiple of 2).
std::optional<Pdu> buildWriteMultipleRegisters(int startAddress, int quantity,
                                               const QByteArray& data,
                                               QString* errorOut = nullptr);

} // namespace pdu_builder

} // namespace modbus::base
