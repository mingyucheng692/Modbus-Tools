/**
 * @file ModbusAddressMapping.h
 * @brief Modbus address mapping and translation between 0-based PDU and 1-based/PLC addresses.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <cstdint>
#include <optional>

namespace modbus::address {

/**
 * @brief Address base mode for input and display.
 */
enum class AddressBase {
    Offset0Based = 0,    ///< 0-based relative offset (PDU Address: 0 ~ 65535)
    PlcAddress1Based = 1 ///< 1-based PLC address (1 ~ 65536) or Modicon notation (e.g. 40001, 400001)
};

/**
 * @brief Address mapping result structure.
 */
struct AddressMappingResult {
    uint16_t pduAddress = 0;              ///< Resolved 0-based PDU address (0 ~ 65535)
    bool isValid = false;                 ///< Whether parsing and mapping succeeded
    QString errorMessage;                 ///< Error description if mapping failed
    std::optional<uint8_t> suggestedFunctionCode = std::nullopt; ///< Suggested function code for Modicon prefixes
};

/**
 * @brief Parse and map user input address string to 0-based PDU address.
 * 
 * Supports hex formats (0x... prefix or ...H suffix) and decimal strings.
 * In PlcAddress1Based mode, handles Modicon 5-digit and 6-digit prefixes:
 * - 40001-49999 / 400001-465536 -> Holding Register (FC03)
 * - 30001-39999 / 300001-365536 -> Input Register (FC04)
 * - 10001-19999 / 100001-165536 -> Discrete Input (FC02)
 * - 00001-09999 / 000001-065536 -> Coil (FC01)
 * Plain 1-based values (1 ~ 65536) map to PDU (input - 1).
 * 
 * @param inputAddress The user input string.
 * @param base The current AddressBase mode.
 * @return AddressMappingResult containing resolution status and PDU address.
 */
[[nodiscard]] AddressMappingResult toPduAddress(const QString& inputAddress, AddressBase base);

/**
 * @brief Format 0-based PDU address to display string according to AddressBase mode.
 * 
 * @param pduAddress The 0-based PDU address (0 ~ 65535).
 * @param base Target display address base mode.
 * @param asHex Whether to display as uppercase hexadecimal (0x... format).
 * @return Formatted address string.
 */
[[nodiscard]] QString toDisplayAddress(uint16_t pduAddress, AddressBase base, bool asHex = false);

} // namespace modbus::address
