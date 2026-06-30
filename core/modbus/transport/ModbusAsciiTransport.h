#pragma once

#include "ITransport.h"
#include <mutex>

namespace modbus::transport {

class ModbusAsciiTransport : public ITransport {
public:
    QByteArray buildRequest(const base::Pdu& pdu, uint8_t slaveId) override;
    ParseResponseResult parseResponse(const QByteArray& adu) override;
    int checkIntegrity(const QByteArray& data) override;
    void resetPendingState() override;

private:
    mutable std::mutex pendingMutex_;
    uint8_t expectedResponseSlaveId_ = 0;
    bool hasPendingRequest_ = false;
};

} // namespace modbus::transport
