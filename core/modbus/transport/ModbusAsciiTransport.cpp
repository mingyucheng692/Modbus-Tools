#include "ModbusAsciiTransport.h"

#include "../base/ModbusLrc.h"
#include "../base/ModbusProtocolChecks.h"
#include <spdlog/spdlog.h>

namespace modbus::transport {

QByteArray ModbusAsciiTransport::buildRequest(const base::Pdu& pdu, uint8_t slaveId)
{
    QByteArray binaryAdu;
    binaryAdu.append(static_cast<char>(slaveId));
    binaryAdu.append(static_cast<char>(pdu.functionCode()));
    binaryAdu.append(pdu.data());
    binaryAdu.append(static_cast<char>(base::calculateModbusAsciiLrc(binaryAdu)));

    tracker_.setPending(slaveId);

    QByteArray asciiFrame(":");
    asciiFrame.append(binaryAdu.toHex().toUpper());
    asciiFrame.append("\r\n");
    return asciiFrame;
}

ParseResponseResult ModbusAsciiTransport::parseResponse(const QByteArray& adu)
{
    base::AsciiAduFields fields;
    if (base::inspectAsciiAdu(adu, &fields) != adu.size()) {
        spdlog::debug("AsciiTransport: reject reason=lrc_mismatch frameLen={}", adu.size());
        return {ParseResponseStatus::Invalid, std::nullopt};
    }

    const auto outcome = tracker_.check(fields.slaveId);
    if (outcome == PendingSlaveTracker::Outcome::NoPending) {
        return {ParseResponseStatus::Unmatched, std::nullopt};
    }
    if (outcome == PendingSlaveTracker::Outcome::SlaveMismatch) {
        spdlog::debug("AsciiTransport: reject reason=slave_mismatch expected={} actual={}",
                      tracker_.expectedSlaveId(), fields.slaveId);
        return {ParseResponseStatus::Unmatched, std::nullopt};
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
    tracker_.reset();
}

} // namespace modbus::transport
