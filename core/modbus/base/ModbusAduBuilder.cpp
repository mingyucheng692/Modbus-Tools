// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusAduBuilder.h"
#include "ModbusCrc.h"
#include "ModbusLrc.h"
#include "ModbusPduBuilder.h"
#include "modbus/parser/ModbusFrameParser.h"
#include <QCoreApplication>
#include <QtGlobal>

namespace modbus::base {

namespace {

// IEC 61158 wire caps: PDU <= 253B, so RTU/ASCII ADU <= 256B and the TCP
// payload (unit + PDU) fits the 254B MBAP length ceiling.
constexpr qsizetype kMaxPduBytes = 253;

} // namespace

QByteArray buildRtuAdu(uint8_t slaveId, const Pdu& pdu) {
    const QByteArray pduBytes = pdu.toByteArray();
    if (pduBytes.size() > kMaxPduBytes) {
        Q_ASSERT(false && "RTU PDU exceeds the 253-byte Modbus limit");
        return {};
    }
    QByteArray adu;
    adu.reserve(1 + pduBytes.size() + 2);
    adu.append(static_cast<char>(slaveId));
    adu.append(pduBytes);
    appendModbusRtuCrc(adu);
    return adu;
}

QByteArray buildTcpAdu(uint8_t unitId, const Pdu& pdu, uint16_t transactionId) {
    const QByteArray pduBytes = pdu.toByteArray();
    if (pduBytes.size() > kMaxPduBytes) {
        Q_ASSERT(false && "TCP PDU exceeds the 253-byte Modbus limit");
        return {};
    }
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
    if (pduBytes.size() > kMaxPduBytes) {
        Q_ASSERT(false && "ASCII PDU exceeds the 253-byte Modbus limit");
        return {};
    }
    QByteArray raw;
    raw.reserve(1 + pduBytes.size() + 1);
    raw.append(static_cast<char>(slaveId));
    raw.append(pduBytes);
    appendModbusAsciiLrc(raw);

    QByteArray adu;
    adu.reserve(1 + raw.size() * 2 + 2);
    adu.append(':');
    adu.append(raw.toHex().toUpper());
    adu.append("\r\n");
    return adu;
}

QByteArray buildRequestFramePreview(
    ::modbus::parser::ProtocolType protocol,
    uint8_t slaveId,
    uint8_t fc,
    uint16_t startAddr,
    uint16_t qty,
    const QByteArray& writeData,
    QString* errorOut) {
    const auto fcode = static_cast<FunctionCode>(fc);
    std::optional<Pdu> pduOpt;
    if (fc == 0x01 || fc == 0x02 || fc == 0x03 || fc == 0x04) {
        pduOpt = pdu_builder::buildReadRequest(fcode, startAddr, qty, errorOut);
    } else if (fc == 0x05 || fc == 0x06 || fc == 0x0F || fc == 0x10) {
        pduOpt = pdu_builder::buildWriteRequest(fcode, startAddr, qty, writeData, errorOut);
    } else {
        if (errorOut) {
            *errorOut = QCoreApplication::translate("ModbusAduBuilder",
                "Unsupported function code 0x%1").arg(fc, 2, 16, QLatin1Char('0'));
        }
        return {};
    }

    if (!pduOpt.has_value()) {
        return {};
    }

    switch (protocol) {
    case ::modbus::parser::ProtocolType::Rtu:
        return buildRtuAdu(slaveId, *pduOpt);
    case ::modbus::parser::ProtocolType::Ascii:
        return buildAsciiAdu(slaveId, *pduOpt);
    case ::modbus::parser::ProtocolType::Tcp:
    default:
        return buildTcpAdu(slaveId, *pduOpt, 0);
    }
}

} // namespace modbus::base
