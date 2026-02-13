#pragma once

#include "ITransport.h"
#include <QByteArray>

namespace modbus::transport {

class TcpTransport : public ITransport {
public:
    TcpTransport();
    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    std::optional<base::Pdu> parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;

private:
    uint16_t transactionId_;
};

} // namespace modbus::transport
