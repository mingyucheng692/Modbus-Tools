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
#include <QCoreApplication>
#include <QtEndian>

namespace modbus::base {

namespace {

constexpr auto kProtocolChecksContext = "ModbusProtocolChecks";
constexpr auto kOperationException = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Exception");
constexpr auto kOperationBitRead = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Bit read");
constexpr auto kOperationRegisterRead = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Register read");
constexpr auto kOperationWriteSingle = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Write single");
constexpr auto kOperationWriteMultiple = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Write multiple");
constexpr auto kPayloadMismatchText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "%1 response payload length mismatch: expected %2, got %3");
constexpr auto kExceptionFunctionCodeMismatchText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Exception function code does not match request");
constexpr auto kResponseFunctionCodeMismatchText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Response function code does not match request");
constexpr auto kBitReadRequestQuantityMissingText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Request quantity missing for bit-read validation");
constexpr auto kBitReadPayloadLengthMismatchText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Bit-read response byte count does not match payload length");
constexpr auto kBitReadQuantityMismatchText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Bit-read response byte count does not match requested quantity");
constexpr auto kRegisterReadRequestQuantityMissingText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Request quantity missing for register-read validation");
constexpr auto kRegisterReadPayloadLengthMismatchText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Register-read response byte count does not match payload length");
constexpr auto kRegisterReadQuantityMismatchText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Register-read response byte count does not match requested quantity");
constexpr auto kRegisterReadByteCountEvenText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Register-read response byte count must be even");
constexpr auto kWriteSingleEchoMismatchText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Write-single response echo does not match request");
constexpr auto kWriteMultipleRequestEchoMissingText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Request echo fields missing for write-multiple validation");
constexpr auto kWriteMultipleEchoIncompleteText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Write-multiple response echo fields are incomplete");
constexpr auto kWriteMultipleEchoMismatchText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Write-multiple response echo does not match request");
constexpr auto kUnsupportedFunctionValidationText = QT_TRANSLATE_NOOP("ModbusProtocolChecks", "Unsupported function code for response validation");

bool readBigEndianUInt16(QByteArrayView data, qsizetype offset, uint16_t& value)
{
    if (offset < 0 || offset + 2 > data.size()) {
        return false;
    }

    value = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(data.data() + offset));
    return true;
}

QString trProtocolCheck(const char* sourceText)
{
    return QCoreApplication::translate(kProtocolChecksContext, sourceText);
}

QString payloadLengthMismatch(const char* operation, qsizetype actual, qsizetype expected)
{
    return trProtocolCheck(kPayloadMismatchText)
        .arg(trProtocolCheck(operation))
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
            return trProtocolCheck(kExceptionFunctionCodeMismatchText);
        }
        if (response.data().size() != 1) {
            return payloadLengthMismatch(kOperationException, response.data().size(), 1);
        }
        return QString();
    }

    if (response.functionCode() != request.functionCode()) {
        return trProtocolCheck(kResponseFunctionCodeMismatchText);
    }

    const QByteArrayView requestData(request.data());
    const QByteArrayView responseData(response.data());
    uint16_t requestStartAddress = 0;
    uint16_t requestQuantity = 0;

    switch (request.functionCode()) {
    case FunctionCode::ReadCoils:
    case FunctionCode::ReadDiscreteInputs: {
        if (!readBigEndianUInt16(requestData, 2, requestQuantity)) {
            return trProtocolCheck(kBitReadRequestQuantityMissingText);
        }
        if (responseData.size() < 1) {
            return payloadLengthMismatch(kOperationBitRead, responseData.size(), 1);
        }
        const uint8_t byteCount = static_cast<uint8_t>(responseData[0]);
        if (responseData.size() != 1 + byteCount) {
            return trProtocolCheck(kBitReadPayloadLengthMismatchText);
        }
        const int expectedByteCount = (static_cast<int>(requestQuantity) + 7) / 8;
        if (byteCount != expectedByteCount) {
            return trProtocolCheck(kBitReadQuantityMismatchText);
        }
        return QString();
    }
    case FunctionCode::ReadHoldingRegisters:
    case FunctionCode::ReadInputRegisters: {
        if (!readBigEndianUInt16(requestData, 2, requestQuantity)) {
            return trProtocolCheck(kRegisterReadRequestQuantityMissingText);
        }
        if (responseData.size() < 1) {
            return payloadLengthMismatch(kOperationRegisterRead, responseData.size(), 1);
        }
        const uint8_t byteCount = static_cast<uint8_t>(responseData[0]);
        if (responseData.size() != 1 + byteCount) {
            return trProtocolCheck(kRegisterReadPayloadLengthMismatchText);
        }
        const int expectedByteCount = static_cast<int>(requestQuantity) * 2;
        if (byteCount != expectedByteCount) {
            return trProtocolCheck(kRegisterReadQuantityMismatchText);
        }
        if ((byteCount % 2) != 0) {
            return trProtocolCheck(kRegisterReadByteCountEvenText);
        }
        return QString();
    }
    case FunctionCode::WriteSingleCoil:
    case FunctionCode::WriteSingleRegister:
        if (responseData.size() != requestData.size()) {
            return payloadLengthMismatch(kOperationWriteSingle, responseData.size(), requestData.size());
        }
        if (responseData != requestData) {
            return trProtocolCheck(kWriteSingleEchoMismatchText);
        }
        return QString();
    case FunctionCode::WriteMultipleCoils:
    case FunctionCode::WriteMultipleRegisters:
        if (!readBigEndianUInt16(requestData, 0, requestStartAddress) ||
            !readBigEndianUInt16(requestData, 2, requestQuantity)) {
            return trProtocolCheck(kWriteMultipleRequestEchoMissingText);
        }
        if (responseData.size() != 4) {
            return payloadLengthMismatch(kOperationWriteMultiple, responseData.size(), 4);
        }
        {
            uint16_t responseStartAddress = 0;
            uint16_t responseQuantity = 0;
            if (!readBigEndianUInt16(responseData, 0, responseStartAddress) ||
                !readBigEndianUInt16(responseData, 2, responseQuantity)) {
                return trProtocolCheck(kWriteMultipleEchoIncompleteText);
            }
            if (responseStartAddress != requestStartAddress || responseQuantity != requestQuantity) {
                return trProtocolCheck(kWriteMultipleEchoMismatchText);
            }
        }
        return QString();
    default:
        return trProtocolCheck(kUnsupportedFunctionValidationText);
    }
}

} // namespace modbus::base
