#include "ModbusTcpTransport.h"
#include <QtEndian>

namespace modbus::transport {

ModbusTcpTransport::ModbusTcpTransport() : transactionId_(0) {}

QByteArray ModbusTcpTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId) {
    QByteArray adu;
    uint16_t transactionId = transactionId_++;
    expectedResponseTransactionId_ = transactionId;
    expectedResponseUnitId_ = slaveId;
    hasPendingRequest_ = true;
    uint16_t length = static_cast<uint16_t>(1 + 1 + pdu.data().size());
    adu.append(static_cast<char>((transactionId >> 8) & 0xFF));
    adu.append(static_cast<char>(transactionId & 0xFF));
    adu.append(static_cast<char>(0x00));
    adu.append(static_cast<char>(0x00));
    adu.append(static_cast<char>((length >> 8) & 0xFF));
    adu.append(static_cast<char>(length & 0xFF));
    adu.append(static_cast<char>(slaveId));
    adu.append(static_cast<char>(pdu.functionCode()));
    adu.append(pdu.data());

    return adu;
}

ParseResponseResult ModbusTcpTransport::parseResponse(const QByteArray& adu) {
    if (adu.size() < 8) {
        return {ParseResponseStatus::Invalid, std::nullopt};
    }

    const uint16_t transactionId = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(adu.constData()));
    const uint16_t protocolId = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(adu.constData() + 2));
    const uint16_t length = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(adu.constData() + 4));
    if (protocolId != 0) {
        return {ParseResponseStatus::Invalid, std::nullopt};
    }
    if (length < 2) {
        return {ParseResponseStatus::Invalid, std::nullopt};
    }
    const int fullLength = 6 + static_cast<int>(length);
    if (adu.size() < fullLength) {
        return {ParseResponseStatus::Invalid, std::nullopt};
    }
    if (hasPendingRequest_ && transactionId != expectedResponseTransactionId_) {
        return {ParseResponseStatus::Unmatched, std::nullopt};
    }
    const uint8_t unitId = static_cast<uint8_t>(adu[6]);
    if (hasPendingRequest_ && unitId != expectedResponseUnitId_) {
        return {ParseResponseStatus::Unmatched, std::nullopt};
    }

    hasPendingRequest_ = false;
    const uint8_t fc = static_cast<uint8_t>(adu[7]);
    QByteArray payload = adu.mid(8, length - 2);

    return {ParseResponseStatus::Ok, base::Pdu(static_cast<base::FunctionCode>(fc), payload)};
}

int ModbusTcpTransport::checkIntegrity(const QByteArray& data) {
    if (data.size() < 6) {
        return 0;
    }

    const uint16_t protocolId = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(data.constData() + 2));
    if (protocolId != 0) {
        return -1;
    }
    const uint16_t length = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(data.constData() + 4));
    if (length < 2 || length > 260) {
        return -1;
    }
    const int fullLength = 6 + static_cast<int>(length);
    
    if (data.size() < fullLength) {
        return 0;
    }
    
    return fullLength;
}

} // namespace modbus::transport
