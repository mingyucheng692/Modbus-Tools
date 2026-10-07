// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "ModbusFrame.h"
#include <QByteArray>
#include <cstdint>

namespace modbus::base {

// Encapsulates a Modbus PDU into an RTU Application Data Unit (ADU).
// Prepends the 1-byte slave/unit ID, follows with the serialized PDU, and
// appends the 2-byte CRC-16 (low byte first, little-endian).
[[nodiscard]] QByteArray buildRtuAdu(uint8_t slaveId, const Pdu& pdu);

// Encapsulates a Modbus PDU into a TCP Application Data Unit (ADU) with MBAP Header.
// Constructs a 7-byte MBAP (Modbus Application Protocol) header:
// - Transaction ID (2 bytes, big-endian)
// - Protocol ID (2 bytes, 0x0000)
// - Length field (2 bytes, big-endian = 1 + PDU byte count)
// - Unit ID (1 byte)
// Followed by the serialized PDU bytes.
[[nodiscard]] QByteArray buildTcpAdu(uint8_t unitId, const Pdu& pdu, uint16_t transactionId = 0);

// Encapsulates a Modbus PDU into an ASCII Application Data Unit (ADU).
// Calculates the Longitudinal Redundancy Check (LRC) over [slaveId + PDU],
// encodes the raw buffer into uppercase hexadecimal ASCII, prepends the ':' start
// character, and appends the "\r\n" CRLF trailer.
[[nodiscard]] QByteArray buildAsciiAdu(uint8_t slaveId, const Pdu& pdu);

} // namespace modbus::base

