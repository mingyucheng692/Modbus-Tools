#pragma once

#include <gmock/gmock.h>
#include "modbus/transport/ITransport.h"

namespace modbus::transport {

class MockTransport : public ITransport {
public:
    MOCK_METHOD(QByteArray, buildRequest, (const base::Pdu& pdu, uint8_t slaveId), (override));
    MOCK_METHOD(ParseResponseResult, parseResponse, (const QByteArray& adu), (override));
    MOCK_METHOD(int, checkIntegrity, (const QByteArray& data), (override));
    MOCK_METHOD(void, resetPendingState, (), (override));
};

} // namespace modbus::transport
