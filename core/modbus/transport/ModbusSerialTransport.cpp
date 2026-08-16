/**
 * @file ModbusSerialTransport.cpp
 * @brief Implementation of ModbusSerialTransport.
 *
 * Merged from ModbusRtuTransport and ModbusAsciiTransport, parameterised
 * by SerialFraming.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusSerialTransport.h"
#include "../base/ModbusCrc.h"
#include "../base/ModbusLrc.h"
#include "../base/ModbusProtocolChecks.h"
#include <QDataStream>
#include <QIODevice>
#include <spdlog/spdlog.h>

namespace modbus::transport {

ModbusSerialTransport::ModbusSerialTransport(SerialFraming framing)
    : framing_(framing)
{
}

QByteArray ModbusSerialTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId) {
    tracker_.setPending(slaveId);

    // --- common binary ADU body (slaveId + functionCode + data) ---
    QByteArray binaryAdu;
    QDataStream stream(&binaryAdu, QIODevice::WriteOnly);
    stream << slaveId;
    stream << static_cast<uint8_t>(pdu.functionCode());
    binaryAdu.append(pdu.data());

    if (framing_ == SerialFraming::Rtu) {
        // RTU: append CRC-16 (little-endian)
        uint16_t crc = base::calculateModbusRtuCrc(binaryAdu);
        binaryAdu.append(static_cast<char>(crc & 0xFF));
        binaryAdu.append(static_cast<char>((crc >> 8) & 0xFF));
        return binaryAdu;
    } else {
        // ASCII: append LRC, then hex-encode with ':' prefix and "\r\n" suffix
        binaryAdu.append(static_cast<char>(base::calculateModbusAsciiLrc(binaryAdu)));

        QByteArray asciiFrame(":");
        asciiFrame.append(binaryAdu.toHex().toUpper());
        asciiFrame.append("\r\n");
        return asciiFrame;
    }
}

ParseResponseResult ModbusSerialTransport::parseResponse(const QByteArray& adu) {
    if (framing_ == SerialFraming::Rtu) {
        base::RtuAduFields fields;
        if (base::inspectRtuAdu(adu, &fields) != adu.size()) {
            SPDLOG_DEBUG("RtuTransport: reject reason=crc_mismatch frameLen={}", adu.size());
            return {ParseResponseStatus::Invalid, std::nullopt};
        }

        const auto outcome = tracker_.check(fields.slaveId);
        if (outcome == PendingSlaveTracker::Outcome::NoPending) {
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }
        if (outcome == PendingSlaveTracker::Outcome::SlaveMismatch) {
            SPDLOG_DEBUG("RtuTransport: reject reason=slave_mismatch expected={} actual={}",
                          tracker_.expectedSlaveId(), fields.slaveId);
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }

        QByteArray payload = adu.mid(2, adu.size() - 4);
        return {ParseResponseStatus::Ok, base::Pdu(static_cast<base::FunctionCode>(fields.functionCode), payload)};
    } else {
        base::AsciiAduFields fields;
        if (base::inspectAsciiAdu(adu, &fields) != adu.size()) {
            SPDLOG_DEBUG("AsciiTransport: reject reason=lrc_mismatch frameLen={}", adu.size());
            return {ParseResponseStatus::Invalid, std::nullopt};
        }

        const auto outcome = tracker_.check(fields.slaveId);
        if (outcome == PendingSlaveTracker::Outcome::NoPending) {
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }
        if (outcome == PendingSlaveTracker::Outcome::SlaveMismatch) {
            SPDLOG_DEBUG("AsciiTransport: reject reason=slave_mismatch expected={} actual={}",
                          tracker_.expectedSlaveId(), fields.slaveId);
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }

        const QByteArray payload = fields.binaryAdu.mid(2, fields.binaryAdu.size() - 3);
        return {
            ParseResponseStatus::Ok,
            base::Pdu(static_cast<base::FunctionCode>(fields.functionCode), payload)
        };
    }
}

int ModbusSerialTransport::checkIntegrity(const QByteArray& data) {
    if (framing_ == SerialFraming::Rtu) {
        // RTU 帧边界由 session 层基于 t3.5 静默时间确定；这里仅做最小长度与 CRC 校验。
        return base::inspectRtuAdu(data);
    } else {
        return base::inspectAsciiAdu(data);
    }
}

void ModbusSerialTransport::resetPendingState() {
    tracker_.reset();
}

} // namespace modbus::transport