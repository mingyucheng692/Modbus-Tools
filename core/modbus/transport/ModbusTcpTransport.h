#pragma once

#include "ITransport.h"
#include <QByteArray>

namespace modbus::transport {

class ModbusTcpTransport : public ITransport {
public:
    ModbusTcpTransport();
    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    std::optional<base::Pdu> parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;

private:
    uint16_t transactionId_;
    uint16_t expectedResponseTransactionId_ = 0;
    bool hasPendingRequest_ = false;
};

} // namespace modbus::transport
