/**
 * @file ModbusAduBuilder.h
 * @brief Pure helper functions to encapsulate Modbus PDUs into complete Application Data Units (ADUs).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ModbusFrame.h"
#include <QByteArray>
#include <cstdint>

namespace modbus::base {

/**
 * @brief Encapsulates a Modbus PDU into an RTU Application Data Unit (ADU).
 *
 * Prepends the 1-byte slave/unit ID, follows with the serialized PDU, and
 * appends the 2-byte CRC-16 (low byte first, little-endian).
 *
 * @param slaveId Modbus RTU slave address (1-247).
 * @param pdu The Modbus Protocol Data Unit to encapsulate.
 * @return Complete RTU frame as QByteArray.
 */
[[nodiscard]] QByteArray buildRtuAdu(uint8_t slaveId, const Pdu& pdu);

/**
 * @brief Encapsulates a Modbus PDU into a TCP Application Data Unit (ADU) with MBAP Header.
 *
 * Constructs a 7-byte MBAP (Modbus Application Protocol) header:
 * - Transaction ID (2 bytes, big-endian)
 * - Protocol ID (2 bytes, 0x0000)
 * - Length field (2 bytes, big-endian = 1 + PDU byte count)
 * - Unit ID (1 byte)
 * Followed by the serialized PDU bytes.
 *
 * @param unitId Modbus TCP unit identifier (1-255).
 * @param pdu The Modbus Protocol Data Unit to encapsulate.
 * @param transactionId Transaction identifier (defaults to 0x0000).
 * @return Complete TCP frame as QByteArray.
 */
[[nodiscard]] QByteArray buildTcpAdu(uint8_t unitId, const Pdu& pdu, uint16_t transactionId = 0);

/**
 * @brief Encapsulates a Modbus PDU into an ASCII Application Data Unit (ADU).
 *
 * Calculates the Longitudinal Redundancy Check (LRC) over [slaveId + PDU],
 * encodes the raw buffer into uppercase hexadecimal ASCII, prepends the ':' start
 * character, and appends the "\r\n" CRLF trailer.
 *
 * @param slaveId Modbus ASCII slave address (1-247).
 * @param pdu The Modbus Protocol Data Unit to encapsulate.
 * @return Complete ASCII frame as QByteArray.
 */
[[nodiscard]] QByteArray buildAsciiAdu(uint8_t slaveId, const Pdu& pdu);

} // namespace modbus::base
