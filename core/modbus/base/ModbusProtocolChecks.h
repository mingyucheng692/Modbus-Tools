/**
 * @file ModbusProtocolChecks.h
 * @brief Header file for ModbusProtocolChecks.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QByteArrayView>
#include <QString>
#include <cstdint>

namespace modbus::base {

class Pdu;

struct TcpAduFields {
    uint16_t transactionId = 0;
    uint16_t protocolId = 0;
    uint16_t length = 0;
    uint8_t unitId = 0;
};

struct RtuAduFields {
    uint8_t slaveId = 0;
    uint8_t functionCode = 0;
    uint16_t receivedCrc = 0;
    uint16_t calculatedCrc = 0;
};

// Returns 0 for incomplete, >0 for a complete frame length, and -1 for invalid data.
int inspectTcpAdu(QByteArrayView adu, TcpAduFields* fields = nullptr);

// Returns 0 for incomplete, >0 for a complete frame length, and -1 for invalid data.
int inspectRtuAdu(QByteArrayView adu, RtuAduFields* fields = nullptr);

// Returns an empty string when the response PDU matches the request semantics.
QString validateResponsePdu(const Pdu& request, const Pdu& response);

} // namespace modbus::base
