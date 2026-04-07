#include "ModbusRtuTransport.h"
#include "../base/ModbusCrc.h"
#include "../base/ModbusProtocolChecks.h"
#include <QDataStream>
#include <QIODevice>

namespace modbus::transport {

QByteArray ModbusRtuTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId) {
    QByteArray adu;
    expectedResponseSlaveId_ = slaveId;
    hasPendingRequest_ = true;
    QDataStream stream(&adu, QIODevice::WriteOnly);
    stream << slaveId;
    stream << static_cast<uint8_t>(pdu.functionCode());
    
    // 写入数据负载
    adu.append(pdu.data());

    // 计算 CRC
    uint16_t crc = base::calculateModbusRtuCrc(adu);
    
    // CRC 低字节在前
    adu.append(static_cast<char>(crc & 0xFF));
    adu.append(static_cast<char>((crc >> 8) & 0xFF));

    return adu;
}

ParseResponseResult ModbusRtuTransport::parseResponse(const QByteArray& adu) {
    base::RtuAduFields fields;
    if (base::inspectRtuAdu(adu, &fields) != adu.size()) {
        return {ParseResponseStatus::Invalid, std::nullopt};
    }

    if (hasPendingRequest_ && fields.slaveId != expectedResponseSlaveId_) {
        return {ParseResponseStatus::Unmatched, std::nullopt};
    }
    hasPendingRequest_ = false;
    QByteArray payload = adu.mid(2, adu.size() - 4);

    return {ParseResponseStatus::Ok, base::Pdu(static_cast<base::FunctionCode>(fields.functionCode), payload)};
}

int ModbusRtuTransport::checkIntegrity(const QByteArray& data) {
    // RTU 帧边界由 session 层基于 t3.5 静默时间确定；这里仅做最小长度与 CRC 校验。
    return base::inspectRtuAdu(data);
}

void ModbusRtuTransport::resetPendingState() {
    hasPendingRequest_ = false;
    expectedResponseSlaveId_ = 0;
}

} // namespace modbus::transport
