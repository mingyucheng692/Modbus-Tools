/**
 * @file ModbusAduBuilder.cpp
 * @brief Implementation of pure helper functions to encapsulate Modbus PDUs into complete ADUs.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusAduBuilder.h"
#include "ModbusCrc.h"
#include "ModbusLrc.h"

namespace modbus::base {

QByteArray buildRtuAdu(uint8_t slaveId, const Pdu& pdu) {
    const QByteArray pduBytes = pdu.toByteArray();
    QByteArray adu;
    adu.reserve(1 + pduBytes.size() + 2);
    adu.append(static_cast<char>(slaveId));
    adu.append(pduBytes);
    const uint16_t crc = calculateModbusRtuCrc(adu);
    adu.append(static_cast<char>(crc & 0xFF));
    adu.append(static_cast<char>((crc >> 8) & 0xFF));
    return adu;
}

QByteArray buildTcpAdu(uint8_t unitId, const Pdu& pdu, uint16_t transactionId) {
    const QByteArray pduBytes = pdu.toByteArray();
    const auto length = static_cast<uint16_t>(1 + pduBytes.size());
    QByteArray adu;
    adu.reserve(7 + pduBytes.size());
    adu.append(static_cast<char>((transactionId >> 8) & 0xFF));
    adu.append(static_cast<char>(transactionId & 0xFF));
    adu.append('\0');
    adu.append('\0');
    adu.append(static_cast<char>((length >> 8) & 0xFF));
    adu.append(static_cast<char>(length & 0xFF));
    adu.append(static_cast<char>(unitId));
    adu.append(pduBytes);
    return adu;
}

QByteArray buildAsciiAdu(uint8_t slaveId, const Pdu& pdu) {
    const QByteArray pduBytes = pdu.toByteArray();
    QByteArray raw;
    raw.reserve(1 + pduBytes.size() + 1);
    raw.append(static_cast<char>(slaveId));
    raw.append(pduBytes);
    const uint8_t lrc = calculateModbusAsciiLrc(raw);
    raw.append(static_cast<char>(lrc));

    QByteArray adu;
    adu.reserve(1 + raw.size() * 2 + 2);
    adu.append(':');
    adu.append(raw.toHex().toUpper());
    adu.append("\r\n");
    return adu;
}

} // namespace modbus::base
