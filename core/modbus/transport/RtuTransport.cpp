#include "RtuTransport.h"
#include <QDataStream>
#include <QIODevice>

namespace modbus::transport {

QByteArray RtuTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId) {
    QByteArray adu;
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

std::optional<base::Pdu> RtuTransport::parseResponse(const QByteArray& adu) {
    if (adu.size() < 4) { // 最小长度：地址(1) + 功能码(1) + CRC(2)
        return std::nullopt;
    }

    // 校验 CRC
    uint16_t receivedCrc = static_cast<uint8_t>(adu[adu.size() - 2]) | 
                           (static_cast<uint8_t>(adu[adu.size() - 1]) << 8);
    
    QByteArray dataWithoutCrc = adu.left(adu.size() - 2);
    if (calculateCrc(dataWithoutCrc) != receivedCrc) {
        return std::nullopt; // CRC 错误
    }

    uint8_t slaveId = static_cast<uint8_t>(adu[0]);
    uint8_t fc = static_cast<uint8_t>(adu[1]);
    QByteArray payload = adu.mid(2, adu.size() - 4);

    return base::Pdu(static_cast<base::FunctionCode>(fc), payload);
}

int RtuTransport::checkIntegrity(const QByteArray& data) {
    if (data.size() < 4) {
        return 0; // 不完整
    }
    
    // RTU 协议本身没有长度字段，通常依赖超时断帧。
    // 如果必须做完整性检查，需要解析功能码和数据长度。
    // 这里简单实现：假设数据流是完整的响应帧（通常由 io 层通过超时分包）
    // 或者可以根据功能码预判长度，但这需要详细协议表。
    // 暂时：只要 CRC 校验通过就算完整，否则如果长度够大但 CRC 错则丢弃
    
    // 简易策略：检查最后两字节是否匹配 CRC
    uint16_t receivedCrc = static_cast<uint8_t>(data[data.size() - 2]) | 
                           (static_cast<uint8_t>(data[data.size() - 1]) << 8);
    QByteArray dataWithoutCrc = data.left(data.size() - 2);
    
    if (calculateCrc(dataWithoutCrc) == receivedCrc) {
        return data.size();
    }

    // CRC 不对，可能是数据还没收完，也可能是错包。
    // 这是一个难点。通常 RTU 依赖 silent interval。
    // 假设 io 层已经按照 silent interval 分包，这里只负责校验。
    return 0; 
}

uint16_t RtuTransport::calculateCrc(const QByteArray& data) {
    uint16_t crc = 0xFFFF;
    for (char c : data) {
        crc ^= static_cast<uint8_t>(c);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

} // namespace modbus::transport
