#pragma once

#include "ITransport.h"
#include <QByteArray>
#include <unordered_map>

namespace modbus::transport {

class ModbusTcpTransport : public ITransport {
public:
    ModbusTcpTransport();
    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    ParseResponseResult parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;
    void resetPendingState() override;
    
    // 清理过期或悬挂的事务表项，避免长期超时积压导致内存泄漏与 ID 冲突
    void purgeStaleTransactions(uint16_t thresholdCount = 256);

private:
    uint16_t transactionId_;
    std::map<uint16_t, uint8_t> pendingTransactions_;
};

} // namespace modbus::transport
