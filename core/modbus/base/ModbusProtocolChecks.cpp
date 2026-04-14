/**
 * @file ModbusProtocolChecks.cpp
 * @brief Implementation of ModbusProtocolChecks.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusProtocolChecks.h"
#include "AppConstants.h"
#include "ModbusCrc.h"
#include "ModbusFrame.h"
#include <QtEndian>

namespace modbus::base {

namespace {

bool readBigEndianUInt16(QByteArrayView data, qsizetype offset, uint16_t& value)
{
    if (offset < 0 || offset + 2 > data.size()) {
        return false;
    }

    value = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(data.data() + offset));
    return true;
}

QString payloadLengthMismatch(const char* operation, qsizetype actual, qsizetype expected)
{
    return QStringLiteral("%1 response payload length mismatch: expected %2, got %3")
        .arg(QString::fromLatin1(operation))
        .arg(expected)
        .arg(actual);
}

} // namespace

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
    if (length < 2 || length > app::constants::Values::Modbus::kMaxTcpMbapLength) {
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
    if (adu.size() < 5) {
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

QString validateResponsePdu(const Pdu& request, const Pdu& response)
{
    if (response.isException()) {
        if (response.originalFunctionCode() != request.functionCode()) {
            return QStringLiteral("Exception function code does not match request");
        }
        if (response.data().size() != 1) {
            return payloadLengthMismatch("Exception", response.data().size(), 1);
        }
        return QString();
    }

    if (response.functionCode() != request.functionCode()) {
        return QStringLiteral("Response function code does not match request");
    }

    const QByteArrayView requestData(request.data());
    const QByteArrayView responseData(response.data());
    uint16_t requestStartAddress = 0;
    uint16_t requestQuantity = 0;

    switch (request.functionCode()) {
    case FunctionCode::ReadCoils:
    case FunctionCode::ReadDiscreteInputs: {
        if (!readBigEndianUInt16(requestData, 2, requestQuantity)) {
            return QStringLiteral("Request quantity missing for bit-read validation");
        }
        if (responseData.size() < 1) {
            return payloadLengthMismatch("Bit read", responseData.size(), 1);
        }
        const uint8_t byteCount = static_cast<uint8_t>(responseData[0]);
        if (responseData.size() != 1 + byteCount) {
            return QStringLiteral("Bit-read response byte count does not match payload length");
        }
        const int expectedByteCount = (static_cast<int>(requestQuantity) + 7) / 8;
        if (byteCount != expectedByteCount) {
            return QStringLiteral("Bit-read response byte count does not match requested quantity");
        }
        return QString();
    }
    case FunctionCode::ReadHoldingRegisters:
    case FunctionCode::ReadInputRegisters: {
        if (!readBigEndianUInt16(requestData, 2, requestQuantity)) {
            return QStringLiteral("Request quantity missing for register-read validation");
        }
        if (responseData.size() < 1) {
            return payloadLengthMismatch("Register read", responseData.size(), 1);
        }
        const uint8_t byteCount = static_cast<uint8_t>(responseData[0]);
        if (responseData.size() != 1 + byteCount) {
            return QStringLiteral("Register-read response byte count does not match payload length");
        }
        const int expectedByteCount = static_cast<int>(requestQuantity) * 2;
        if (byteCount != expectedByteCount) {
            return QStringLiteral("Register-read response byte count does not match requested quantity");
        }
        if ((byteCount % 2) != 0) {
            return QStringLiteral("Register-read response byte count must be even");
        }
        return QString();
    }
    case FunctionCode::WriteSingleCoil:
    case FunctionCode::WriteSingleRegister:
        if (responseData.size() != requestData.size()) {
            return payloadLengthMismatch("Write single", responseData.size(), requestData.size());
        }
        if (responseData != requestData) {
            return QStringLiteral("Write-single response echo does not match request");
        }
        return QString();
    case FunctionCode::WriteMultipleCoils:
    case FunctionCode::WriteMultipleRegisters:
        if (!readBigEndianUInt16(requestData, 0, requestStartAddress) ||
            !readBigEndianUInt16(requestData, 2, requestQuantity)) {
            return QStringLiteral("Request echo fields missing for write-multiple validation");
        }
        if (responseData.size() != 4) {
            return payloadLengthMismatch("Write multiple", responseData.size(), 4);
        }
        {
            uint16_t responseStartAddress = 0;
            uint16_t responseQuantity = 0;
            if (!readBigEndianUInt16(responseData, 0, responseStartAddress) ||
                !readBigEndianUInt16(responseData, 2, responseQuantity)) {
                return QStringLiteral("Write-multiple response echo fields are incomplete");
            }
            if (responseStartAddress != requestStartAddress || responseQuantity != requestQuantity) {
                return QStringLiteral("Write-multiple response echo does not match request");
            }
        }
        return QString();
    default:
        return QStringLiteral("Unsupported function code for response validation");
    }
}

} // namespace modbus::base
