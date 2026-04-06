#include "ModbusProtocolChecks.h"
#include "AppConstants.h"
#include "ModbusCrc.h"
#include <QtEndian>

namespace modbus::base {

int inspectTcpAdu(QByteArrayView adu, TcpAduFields* fields) {
    if (adu.size() < 6) {
        return 0;
    }

    const auto* data = reinterpret_cast<const uchar*>(adu.data());
    const uint16_t transactionId = qFromBigEndian<uint16_t>(data);
    const uint16_t protocolId = qFromBigEndian<uint16_t>(data + 2);
    const uint16_t length = qFromBigEndian<uint16_t>(data + 4);

    if (protocolId != 0) {
        return -1;
    }
    if (length < 2 || length > app::constants::Constants::Modbus::kMaxTcpMbapLength) {
        return -1;
    }

    const int fullLength = 6 + static_cast<int>(length);
    if (adu.size() < fullLength) {
        return 0;
    }

    if (fields) {
        fields->transactionId = transactionId;
        fields->protocolId = protocolId;
        fields->length = length;
        fields->unitId = static_cast<uint8_t>(adu[6]);
    }

    return fullLength;
}

int inspectRtuAdu(QByteArrayView adu, RtuAduFields* fields) {
    if (adu.size() < 4) {
        return 0;
    }

    const auto* data = reinterpret_cast<const uchar*>(adu.data());
    const uint16_t receivedCrc = qFromLittleEndian<uint16_t>(data + adu.size() - 2);
    const uint16_t calculatedCrc = calculateModbusRtuCrc(adu.first(adu.size() - 2));

    if (fields) {
        fields->slaveId = static_cast<uint8_t>(adu[0]);
        fields->functionCode = static_cast<uint8_t>(adu[1]);
        fields->receivedCrc = receivedCrc;
        fields->calculatedCrc = calculatedCrc;
    }

    if (calculatedCrc != receivedCrc) {
        return -1;
    }

    return adu.size();
}

} // namespace modbus::base
