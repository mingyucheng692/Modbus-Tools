#pragma once

#include "ITransport.h"
#include <QByteArray>

namespace modbus::transport {

class ModbusRtuTransport : public ITransport {
public:
    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    ParseResponseResult parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;

private:
    uint16_t calculateCrc(const QByteArray& data);
    uint8_t expectedResponseSlaveId_ = 0;
    bool hasPendingRequest_ = false;
};

} // namespace modbus::transport
