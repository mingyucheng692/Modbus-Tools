#include "ModbusRtuTransport.h"
#include "../base/ModbusCrc.h"
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
    uint16_t crc = calculateCrc(adu);
    
    // CRC 低字节在前
    adu.append(static_cast<char>(crc & 0xFF));
    adu.append(static_cast<char>((crc >> 8) & 0xFF));

    return adu;
}

ParseResponseResult ModbusRtuTransport::parseResponse(const QByteArray& adu) {
    if (adu.size() < 4) { // 最小长度：地址(1) + 功能码(1) + CRC(2)
        return {ParseResponseStatus::Invalid, std::nullopt};
    }

    // 校验 CRC
    uint16_t receivedCrc = static_cast<uint8_t>(adu[adu.size() - 2]) | 
                           (static_cast<uint8_t>(adu[adu.size() - 1]) << 8);
    
    QByteArray dataWithoutCrc = adu.left(adu.size() - 2);
    if (calculateCrc(dataWithoutCrc) != receivedCrc) {
        return {ParseResponseStatus::Invalid, std::nullopt};
    }

    uint8_t slaveId = static_cast<uint8_t>(adu[0]);
    if (hasPendingRequest_ && slaveId != expectedResponseSlaveId_) {
        return {ParseResponseStatus::Unmatched, std::nullopt};
    }
    hasPendingRequest_ = false;
    uint8_t fc = static_cast<uint8_t>(adu[1]);
    QByteArray payload = adu.mid(2, adu.size() - 4);

    return {ParseResponseStatus::Ok, base::Pdu(static_cast<base::FunctionCode>(fc), payload)};
}

int ModbusRtuTransport::checkIntegrity(const QByteArray& data) {
    if (data.size() < 4) {
        return 0; // 不完整
    }

    // RTU 帧边界由 session 层基于 t3.5 静默时间确定；这里仅做最小长度与 CRC 校验。
    uint16_t receivedCrc = static_cast<uint8_t>(data[data.size() - 2]) |
                           (static_cast<uint8_t>(data[data.size() - 1]) << 8);
    QByteArray dataWithoutCrc = data.left(data.size() - 2);
    if (calculateCrc(dataWithoutCrc) != receivedCrc) {
        return -1;
    }

    return data.size();
}

uint16_t ModbusRtuTransport::calculateCrc(const QByteArray& data) {
    return base::calculateModbusRtuCrc(data);
}

} // namespace modbus::transport
