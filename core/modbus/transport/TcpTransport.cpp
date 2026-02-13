#include "TcpTransport.h"
#include <QDataStream>
#include <QtEndian>
#include <QIODevice>

namespace modbus::transport {

TcpTransport::TcpTransport() : transactionId_(0) {}

QByteArray TcpTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId) {
    QByteArray adu;
    QDataStream stream(&adu, QIODevice::WriteOnly);
    
    // Modbus TCP Header (MBAP)
    // Transaction Identifier (2 Bytes)
    stream << qToBigEndian(transactionId_++);
    
    // Protocol Identifier (2 Bytes) - 0 for Modbus
    stream << qToBigEndian(static_cast<uint16_t>(0));
    
    // Length (2 Bytes) - Number of following bytes (Unit Identifier + PDU)
    uint16_t length = 1 + 1 + pdu.data().size(); // UnitID(1) + FC(1) + Data
    stream << qToBigEndian(length);
    
    // Unit Identifier (1 Byte)
    stream << slaveId;
    
    // PDU
    stream << static_cast<uint8_t>(pdu.functionCode());
    adu.append(pdu.data());

    return adu;
}

std::optional<base::Pdu> TcpTransport::parseResponse(const QByteArray& adu) {
    if (adu.size() < 7) { // 最小 MBAP 长度
        return std::nullopt;
    }

    // 解析 MBAP
    // 简化处理：暂时忽略 Transaction ID 校验，假设是同步请求
    
    uint16_t length = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(adu.data() + 4));
    
    if (adu.size() < 6 + length) {
        return std::nullopt; // 数据未收全
    }

    // MBAP(7 bytes) = TransID(2) + ProtoID(2) + Len(2) + UnitID(1)
    // 实际上 Length 字段包含 UnitID。
    // ADU 结构: [MBAP Header 7 bytes] [PDU]
    // MBAP: [TransID 2] [ProtoID 2] [Length 2] [UnitID 1]
    // 注意：Length 字段的值 = UnitID(1) + PDU 长度
    
    // 修正：UnitID 是 MBAP 的最后一个字节，也是 Length 计数的一部分
    uint8_t unitId = static_cast<uint8_t>(adu[6]);
    uint8_t fc = static_cast<uint8_t>(adu[7]);
    
    // PDU 数据部分
    if (length < 2) {
        return std::nullopt;
    }
    QByteArray payload = adu.mid(8, length - 2); // Length - UnitID(1) - FC(1)

    return base::Pdu(static_cast<base::FunctionCode>(fc), payload);
}

int TcpTransport::checkIntegrity(const QByteArray& data) {
    if (data.size() < 6) {
        return 0;
    }

    uint16_t length = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(data.data() + 4));
    int fullLength = 6 + length;
    
    if (data.size() < fullLength) {
        return 0;
    }
    
    return fullLength;
}

} // namespace modbus::transport
