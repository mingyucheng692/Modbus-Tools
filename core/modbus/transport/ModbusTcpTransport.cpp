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
#include "../../Config.h"
#include <QtEndian>
#include <spdlog/spdlog.h>

namespace modbus::transport {

ModbusTcpTransport::ModbusTcpTransport() : transactionId_(0) {}

QByteArray ModbusTcpTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId) {
    QByteArray adu;
    uint16_t transactionId = 0;
    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        transactionId = transactionId_++;

        // 如果累积过多挂起事务，主动淘汰最老的，防止极端情况下 OOM 及 ID 碰撞
        purgeStaleTransactions();

        pendingTransactions_[transactionId] = slaveId;
    }
    uint16_t length = static_cast<uint16_t>(1 + 1 + pdu.data().size());
    char header[6];
    qToBigEndian(transactionId, header);
    qToBigEndian(uint16_t(0), header + 2);
    qToBigEndian(length, header + 4);
    adu.append(header, 6);
    adu.append(static_cast<char>(slaveId));
    adu.append(static_cast<char>(pdu.functionCode()));
    adu.append(pdu.data());

    return adu;
}

ParseResponseResult ModbusTcpTransport::parseResponse(const QByteArray& adu) {
    base::TcpAduFields fields;
    if (base::inspectTcpAdu(adu, &fields) != adu.size()) {
        spdlog::warn("ModbusTcpTransport: invalid TCP ADU, size={}, expected header=6",
                     adu.size());
        return {ParseResponseStatus::Invalid, std::nullopt};
    }

    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        const auto pendingIt = pendingTransactions_.find(fields.transactionId);
        if (pendingIt == pendingTransactions_.end()) {
            spdlog::debug("ModbusTcpTransport: unmatched transactionId={}, no pending entry",
                          fields.transactionId);
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }
        if (fields.unitId != pendingIt->second) {
            spdlog::debug("ModbusTcpTransport: unmatched unitId={}, expected={}",
                          fields.unitId, pendingIt->second);
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }

        pendingTransactions_.erase(pendingIt);
    }
    const uint8_t fc = static_cast<uint8_t>(adu[7]);
    QByteArray payload = adu.mid(8, fields.length - 2);

    return {ParseResponseStatus::Ok, base::Pdu(static_cast<base::FunctionCode>(fc), payload)};
}

int ModbusTcpTransport::checkIntegrity(const QByteArray& data) {
    // fields is an output parameter of inspectTcpAdu; its members are validated
    // internally and not re-checked here, so it is declared but not read.
    base::TcpAduFields fields;
    return base::inspectTcpAdu(data, &fields);
}

void ModbusTcpTransport::resetPendingState() {
    std::lock_guard<std::mutex> lock(pendingMutex_);
    pendingTransactions_.clear();
}

void ModbusTcpTransport::purgeStaleTransactions(uint16_t thresholdCount) {
    // Caller must hold pendingMutex_.
    if (pendingTransactions_.size() > thresholdCount) {
        // 保留最新的 (thresholdCount / 2) 个事务，清理过旧事务
        size_t keepCount = thresholdCount / 2;
        auto it = pendingTransactions_.end();
        std::advance(it, -keepCount);
        pendingTransactions_.erase(pendingTransactions_.begin(), it);
    }
}

} // namespace modbus::transport
