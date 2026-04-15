/**
 * @file ModbusDataHelper.h
 * @brief Header file for ModbusDataHelper.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QByteArray>
#include <QString>
#include <QList>

namespace modbus::base {

/**
 * @brief Utility for parsing data formats from UI strings.
 * 
 * Provides centralized parsing logic for HEX, Decimal, and Binary strings
 * to ensure consistency across different views and adhere to protocol-specific
 * byte ordering constraints.
 */
class ModbusDataHelper {
public:
    /**
     * @brief Parses a HEX string into a QByteArray.
     * 
     * Ignores non-HEX characters. Prepends a '0' if the hex-digit count is odd.
     * @param input The input HEX string (e.g., "01 23 AB").
     * @return Packed byte array.
     */
    static QByteArray parseHex(const QString& input);

    /**
     * @brief Parses a list of decimal numbers into a QByteArray as 16-bit registers.
     * 
     * Input can be space, comma, or semi-colon separated.
     * @param input The input decimal string (e.g., "123, 456").
     * @param ok Set to true if parsing succeeded, false if any value is out of range (0-65535).
     * @return Packed byte array (Big-Endian).
     */
    static QByteArray parseDecimalList(const QString& input, bool& ok);

    /**
     * @brief Parses a string of bits ('0' and '1') into a QByteArray.
     * 
     * Packed using LSB-first within each byte (Modbus standard for coils).
     * @param input The bit string (e.g., "1 1 0 1").
     * @return Packed byte array.
     */
    static QByteArray parseBinary(const QString& input);

    /**
     * @brief Parses a string into an integer with smart format detection.
     * 
     * Supports:
     * - HEX: 0x or 0X prefix (e.g., "0x12")
     * - HEX: H or h suffix (e.g., "12H")
     * - DEC: Standard decimal string (e.g., "18")
     * 
     * @note 0x prefix and H suffix are mutually exclusive.
     * 
     * @param input The input string.
     * @param ok Set to true if parsing succeeded and format is valid, false otherwise.
     * @return The parsed integer value, or 0 if parsing failed.
     */
    static int parseSmartInt(const QString& input, bool* ok = nullptr);
};

} // namespace modbus::base
