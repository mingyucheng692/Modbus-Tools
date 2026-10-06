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
#include "../base/ModbusAduBuilder.h"
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
    pendingSlave_ = slaveId;

    if (framing_ == SerialFraming::Rtu) {
        return base::buildRtuAdu(slaveId, pdu);
    }
    return base::buildAsciiAdu(slaveId, pdu);
}

ParseResponseResult ModbusSerialTransport::parseResponse(const QByteArray& adu) {
    if (framing_ == SerialFraming::Rtu) {
        base::RtuAduFields fields;
        if (base::inspectRtuAdu(adu, &fields) != adu.size()) {
            SPDLOG_DEBUG("RtuTransport: reject reason=crc_mismatch frameLen={}", adu.size());
            return {ParseResponseStatus::Invalid, std::nullopt};
        }

        if (!pendingSlave_.has_value()) {
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }
        if (*pendingSlave_ != fields.slaveId) {
            SPDLOG_DEBUG("RtuTransport: reject reason=slave_mismatch expected={} actual={}",
                          *pendingSlave_, fields.slaveId);
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }
        pendingSlave_.reset();

        const qsizetype payloadLen = adu.size() - 4;
        const QByteArrayView payload = (payloadLen > 0)
            ? QByteArrayView(adu).sliced(2, payloadLen)
            : QByteArrayView();
        return {ParseResponseStatus::Ok, base::Pdu(static_cast<base::FunctionCode>(fields.functionCode), payload)};
    } else {
        base::AsciiAduFields fields;
        if (base::inspectAsciiAdu(adu, &fields) != adu.size()) {
            SPDLOG_DEBUG("AsciiTransport: reject reason=lrc_mismatch frameLen={}", adu.size());
            return {ParseResponseStatus::Invalid, std::nullopt};
        }

        if (!pendingSlave_.has_value()) {
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }
        if (*pendingSlave_ != fields.slaveId) {
            SPDLOG_DEBUG("AsciiTransport: reject reason=slave_mismatch expected={} actual={}",
                          *pendingSlave_, fields.slaveId);
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }
        pendingSlave_.reset();

        const qsizetype payloadLen = fields.binaryAdu.size() - 3;
        const QByteArrayView payload = (payloadLen > 0)
            ? QByteArrayView(fields.binaryAdu).sliced(2, payloadLen)
            : QByteArrayView();
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
    pendingSlave_.reset();
}

} // namespace modbus::transport