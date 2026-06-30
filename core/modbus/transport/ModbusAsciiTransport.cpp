#include "ModbusAsciiTransport.h"

#include "../base/ModbusLrc.h"
#include "../base/ModbusProtocolChecks.h"

namespace modbus::transport {

QByteArray ModbusAsciiTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId)
{
    QByteArray binaryAdu;
    binaryAdu.append(static_cast<char>(slaveId));
    binaryAdu.append(static_cast<char>(pdu.functionCode()));
    binaryAdu.append(pdu.data());
    binaryAdu.append(static_cast<char>(base::calculateModbusAsciiLrc(binaryAdu)));

    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        expectedResponseSlaveId_ = slaveId;
        hasPendingRequest_ = true;
    }

    QByteArray asciiFrame(":");
    asciiFrame.append(binaryAdu.toHex().toUpper());
    asciiFrame.append("\r\n");
    return asciiFrame;
}

ParseResponseResult ModbusAsciiTransport::parseResponse(const QByteArray& adu)
{
    base::AsciiAduFields fields;
    if (base::inspectAsciiAdu(adu, &fields) != adu.size()) {
        return {ParseResponseStatus::Invalid, std::nullopt};
    }

    {
        std::lock_guard<std::mutex> lock(pendingMutex_);
        if (!hasPendingRequest_) {
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }
        if (fields.slaveId != expectedResponseSlaveId_) {
            return {ParseResponseStatus::Unmatched, std::nullopt};
        }
        hasPendingRequest_ = false;
        expectedResponseSlaveId_ = 0;
    }

    const QByteArray payload = fields.binaryAdu.mid(2, fields.binaryAdu.size() - 3);
    return {
        ParseResponseStatus::Ok,
        base::Pdu(static_cast<base::FunctionCode>(fields.functionCode), payload)
    };
}

int ModbusAsciiTransport::checkIntegrity(const QByteArray& data)
{
    return base::inspectAsciiAdu(data);
}

void ModbusAsciiTransport::resetPendingState()
{
    std::lock_guard<std::mutex> lock(pendingMutex_);
    hasPendingRequest_ = false;
    expectedResponseSlaveId_ = 0;
}

} // namespace modbus::transport
