/**
 * @file RequestValidator.cpp
 * @brief Implementation of RequestValidator.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "RequestValidator.h"
#include "../base/ModbusFrame.h"
#include "AppConstants.h"
#include <QtEndian>

namespace modbus::session {

bool RequestValidator::readBigEndianUInt16(const QByteArray& data, int offset, uint16_t& out)
{
    if (offset < 0 || offset + 2 > data.size()) {
        return false;
    }
    out = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar*>(data.constData() + offset));
    return true;
}

bool RequestValidator::isAddressRangeValid(uint16_t startAddress, uint16_t quantity)
{
    if (quantity == 0) {
        return false;
    }
    const uint32_t endAddress = static_cast<uint32_t>(startAddress) + static_cast<uint32_t>(quantity) - 1U;
    return endAddress <= static_cast<uint32_t>(app::constants::Values::Modbus::kMaxAddress);
}

RequestValidator::Result RequestValidator::validate(const base::Pdu& request,
    int slaveId, base::ModbusMode mode) const
{
    if (mode == base::ModbusMode::RTU &&
        (slaveId < app::constants::Values::Modbus::kMinSlaveId ||
         slaveId > app::constants::Values::Modbus::kMaxSlaveId)) {
        return {false, QString("Invalid RTU slave id: %1").arg(slaveId)};
    }

    const QByteArray data = request.data();
    if (data.size() > app::constants::Values::Modbus::kMaxPduDataLength) {
        return {false, QString("PDU data too long: %1 bytes").arg(data.size())};
    }

    uint16_t quantity = 0;
    uint16_t startAddress = 0;
    uint16_t value = 0;
    uint8_t declaredByteCount = 0;
    switch (request.functionCode()) {
    case base::FunctionCode::ReadCoils:
    case base::FunctionCode::ReadDiscreteInputs:
    case base::FunctionCode::ReadHoldingRegisters:
    case base::FunctionCode::ReadInputRegisters:
        if (data.size() != 4 ||
            !readBigEndianUInt16(data, 0, startAddress) ||
            !readBigEndianUInt16(data, 2, quantity)) {
            return {false, QString("Invalid read request payload length: %1").arg(data.size())};
        }
        if (quantity < app::constants::Values::Modbus::kMinQuantity) {
            return {false, QString("Read quantity must be at least %1")
                .arg(app::constants::Values::Modbus::kMinQuantity)};
        }
        if (!isAddressRangeValid(startAddress, quantity)) {
            return {false, QString("Read address range exceeds limit: start=%1 quantity=%2")
                .arg(startAddress)
                .arg(quantity)};
        }
        if ((request.functionCode() == base::FunctionCode::ReadCoils ||
             request.functionCode() == base::FunctionCode::ReadDiscreteInputs) &&
            quantity > app::constants::Values::Modbus::kMaxReadBitsQuantity) {
            return {false, QString("Read bit quantity exceeds limit: %1").arg(quantity)};
        }
        if ((request.functionCode() == base::FunctionCode::ReadHoldingRegisters ||
             request.functionCode() == base::FunctionCode::ReadInputRegisters) &&
            quantity > app::constants::Values::Modbus::kMaxReadQuantity) {
            return {false, QString("Read register quantity exceeds limit: %1").arg(quantity)};
        }
        break;
    case base::FunctionCode::WriteSingleCoil:
    case base::FunctionCode::WriteSingleRegister:
        if (data.size() != 4 ||
            !readBigEndianUInt16(data, 0, startAddress) ||
            !readBigEndianUInt16(data, 2, value)) {
            return {false, QString("Invalid write-single request payload length: %1").arg(data.size())};
        }
        if (!isAddressRangeValid(startAddress, 1)) {
            return {false, QString("Write address out of range: %1").arg(startAddress)};
        }
        if (request.functionCode() == base::FunctionCode::WriteSingleCoil &&
            value != 0x0000 && value != 0xFF00) {
            return {false, QString("Invalid single coil value: 0x%1")
                .arg(value, 4, 16, QChar('0'))
                .toUpper()};
        }
        break;
    case base::FunctionCode::WriteMultipleCoils:
    case base::FunctionCode::WriteMultipleRegisters:
        if (data.size() < 5 ||
            !readBigEndianUInt16(data, 0, startAddress) ||
            !readBigEndianUInt16(data, 2, quantity)) {
            return {false, QString("Invalid write-multiple request payload length: %1").arg(data.size())};
        }
        declaredByteCount = static_cast<uint8_t>(data[4]);
        if (quantity < app::constants::Values::Modbus::kMinQuantity) {
            return {false, QString("Write quantity must be at least %1")
                .arg(app::constants::Values::Modbus::kMinQuantity)};
        }
        if (!isAddressRangeValid(startAddress, quantity)) {
            return {false, QString("Write address range exceeds limit: start=%1 quantity=%2")
                .arg(startAddress)
                .arg(quantity)};
        }
        if (static_cast<int>(declaredByteCount) != data.size() - 5) {
            return {false, QString("Write request byte count mismatch: declared %1, actual %2")
                .arg(declaredByteCount)
                .arg(data.size() - 5)};
        }
        if (request.functionCode() == base::FunctionCode::WriteMultipleCoils) {
            if (quantity > app::constants::Values::Modbus::kMaxWriteCoilsQuantity) {
                return {false, QString("Write coil quantity exceeds limit: %1").arg(quantity)};
            }
            const int expectedByteCount = (static_cast<int>(quantity) + 7) / 8;
            if (declaredByteCount != expectedByteCount) {
                return {false, QString("Write coil byte count mismatch: declared %1, expected %2")
                    .arg(declaredByteCount)
                    .arg(expectedByteCount)};
            }
        } else {
            if (quantity > app::constants::Values::Modbus::kMaxWriteRegistersQuantity) {
                return {false, QString("Write register quantity exceeds limit: %1").arg(quantity)};
            }
            const int expectedByteCount = static_cast<int>(quantity) * 2;
            if (declaredByteCount != expectedByteCount) {
                return {false, QString("Write register byte count mismatch: declared %1, expected %2")
                    .arg(declaredByteCount)
                    .arg(expectedByteCount)};
            }
        }
        break;
    default:
        return {false, QString("Unsupported function code: 0x%1")
            .arg(static_cast<int>(request.functionCode()), 2, 16, QChar('0'))
            .toUpper()};
    }

    const int tcpMbapLength = 2 + data.size();
    if (tcpMbapLength > app::constants::Values::Modbus::kMaxTcpMbapLength) {
        return {false, QString("TCP MBAP length exceeds limit: %1").arg(tcpMbapLength)};
    }

    const int rtuAduLength = 4 + data.size();
    if (rtuAduLength > 256) {
        return {false, QString("RTU ADU length exceeds limit: %1").arg(rtuAduLength)};
    }

    return {true, QString()};
}

} // namespace modbus::session