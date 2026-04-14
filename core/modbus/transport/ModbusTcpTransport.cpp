/**
 * @file ModbusTcpTransport.cpp
 * @brief Implementation of ModbusTcpTransport.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusTcpTransport.h"
#include "../base/ModbusProtocolChecks.h"

namespace modbus::transport {

ModbusTcpTransport::ModbusTcpTransport() : transactionId_(0) {}

QByteArray ModbusTcpTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId) {
    QByteArray adu;
    uint16_t transactionId = transactionId_++;
    
    // 如果累积过多挂起事务，主动淘汰最老的，防止极端情况下 OOM 及 ID 碰撞
    purgeStaleTransactions();
    
    pendingTransactions_[transactionId] = slaveId;
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
    base::TcpAduFields fields;
    if (base::inspectTcpAdu(adu, &fields) != adu.size()) {
        return {ParseResponseStatus::Invalid, std::nullopt};
    }

    const auto pendingIt = pendingTransactions_.find(fields.transactionId);
    if (pendingIt == pendingTransactions_.end()) {
        return {ParseResponseStatus::Unmatched, std::nullopt};
    }
    if (fields.unitId != pendingIt->second) {
        return {ParseResponseStatus::Unmatched, std::nullopt};
    }

    pendingTransactions_.erase(pendingIt);
    const uint8_t fc = static_cast<uint8_t>(adu[7]);
    QByteArray payload = adu.mid(8, fields.length - 2);

    return {ParseResponseStatus::Ok, base::Pdu(static_cast<base::FunctionCode>(fc), payload)};
}

int ModbusTcpTransport::checkIntegrity(const QByteArray& data) {
    return base::inspectTcpAdu(data);
}

void ModbusTcpTransport::resetPendingState() {
    pendingTransactions_.clear();
}

void ModbusTcpTransport::purgeStaleTransactions(uint16_t thresholdCount) {
    if (pendingTransactions_.size() > thresholdCount) {
        // 保留最新的 (thresholdCount / 2) 个事务，清理过旧事务
        size_t keepCount = thresholdCount / 2;
        auto it = pendingTransactions_.end();
        std::advance(it, -keepCount);
        pendingTransactions_.erase(pendingTransactions_.begin(), it);
    }
}

} // namespace modbus::transport
