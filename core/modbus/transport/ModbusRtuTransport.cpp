#include "ModbusRtuTransport.h"
#include <QDataStream>
#include <QIODevice>

namespace modbus::transport {

QByteArray ModbusRtuTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId) {
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

std::optional<base::Pdu> ModbusRtuTransport::parseResponse(const QByteArray& adu) {
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

int ModbusRtuTransport::checkIntegrity(const QByteArray& data) {
    if (data.size() < 4) {
        return 0; // 不完整
    }
    
    uint8_t fc = static_cast<uint8_t>(data[1]);
    int expectedLength = 0;
    if ((fc & 0x80) != 0) {
        expectedLength = 5;
    } else if (fc == 0x01 || fc == 0x02 || fc == 0x03 || fc == 0x04) {
        if (data.size() < 3) {
            return 0;
        }
        uint8_t byteCount = static_cast<uint8_t>(data[2]);
        expectedLength = 3 + byteCount + 2;
    } else if (fc == 0x05 || fc == 0x06 || fc == 0x0F || fc == 0x10) {
        expectedLength = 8;
    } else {
        uint16_t receivedCrc = static_cast<uint8_t>(data[data.size() - 2]) |
                               (static_cast<uint8_t>(data[data.size() - 1]) << 8);
        QByteArray dataWithoutCrc = data.left(data.size() - 2);
        if (calculateCrc(dataWithoutCrc) == receivedCrc) {
            return data.size();
        }
        return -1;
    }

    if (data.size() < expectedLength) {
        return 0;
    }

    QByteArray frame = data.left(expectedLength);
    uint16_t receivedCrc = static_cast<uint8_t>(frame[expectedLength - 2]) |
                           (static_cast<uint8_t>(frame[expectedLength - 1]) << 8);
    QByteArray dataWithoutCrc = frame.left(expectedLength - 2);
    if (calculateCrc(dataWithoutCrc) == receivedCrc) {
        return expectedLength;
    }

    return -1;
}

uint16_t ModbusRtuTransport::calculateCrc(const QByteArray& data) {
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
