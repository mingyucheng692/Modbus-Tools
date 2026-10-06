// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "ITransport.h"
#include <QByteArray>
#include <map>
#include <mutex>

namespace modbus::transport {

// Role: Modbus TCP transport implementation managing MBAP headers and transaction ID tracking.
// Thread-safety: Thread-safe (internal std::mutex protects pending transaction map and transaction ID generator).
// Ownership: Owned by session layer (ModbusClient) via std::shared_ptr<ITransport>.
class ModbusTcpTransport : public ITransport {
public:
    ModbusTcpTransport();
    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    ParseResponseResult parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;
    void resetPendingState() override;

private:
    // Evicts stale pending transactions when accumulation exceeds threshold to prevent OOM and ID collision.
    // Caller must hold pendingMutex_ (invoked within buildRequest critical section).
    void purgeStaleTransactions(uint16_t thresholdCount = 256);

    mutable std::mutex pendingMutex_;
    uint16_t transactionId_ = 0;
    std::map<uint16_t, uint8_t> pendingTransactions_;
};

} // namespace modbus::transport
