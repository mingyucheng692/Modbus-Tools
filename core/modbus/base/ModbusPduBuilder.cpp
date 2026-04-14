/**
 * @file ModbusPduBuilder.cpp
 * @brief Implementation of ModbusPduBuilder.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusPduBuilder.h"
#include <QtEndian>
#include <QCoreApplication>

namespace modbus::base {

PduBuildResult ModbusPduBuilder::buildReadRequest(FunctionCode fc, int startAddress, int quantity) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Invalid start address"));
    }
    if (quantity <= 0 || quantity > 2000) {
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Invalid quantity"));
    }

    QByteArray data;
    data.resize(4);
    qToBigEndian<uint16_t>(static_cast<uint16_t>(startAddress), reinterpret_cast<uchar*>(data.data()));
    qToBigEndian<uint16_t>(static_cast<uint16_t>(quantity), reinterpret_cast<uchar*>(data.data() + 2));

    return PduBuildResult::ok(Pdu(fc, data));
}

PduBuildResult ModbusPduBuilder::buildWriteSingleCoil(int startAddress, bool value) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Invalid start address"));
    }

    QByteArray data;
    data.resize(4);
    qToBigEndian<uint16_t>(static_cast<uint16_t>(startAddress), reinterpret_cast<uchar*>(data.data()));
    uint16_t coilValue = value ? 0xFF00 : 0x0000;
    qToBigEndian<uint16_t>(coilValue, reinterpret_cast<uchar*>(data.data() + 2));

    return PduBuildResult::ok(Pdu(FunctionCode::WriteSingleCoil, data));
}

PduBuildResult ModbusPduBuilder::buildWriteSingleRegister(int startAddress, uint16_t value) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Invalid start address"));
    }

    QByteArray data;
    data.resize(4);
    qToBigEndian<uint16_t>(static_cast<uint16_t>(startAddress), reinterpret_cast<uchar*>(data.data()));
    qToBigEndian<uint16_t>(value, reinterpret_cast<uchar*>(data.data() + 2));

    return PduBuildResult::ok(Pdu(FunctionCode::WriteSingleRegister, data));
}

PduBuildResult ModbusPduBuilder::buildWriteMultipleCoils(int startAddress, int quantity, const QByteArray& packedData) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Invalid start address"));
    }
    if (quantity <= 0 || quantity > 1968) { // Modbus limit
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Invalid quantity for 0x0F"));
    }

    int expectedBytes = (quantity + 7) / 8;
    if (packedData.size() != expectedBytes) {
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Data length mismatch for 0x0F"));
    }

    QByteArray data;
    data.reserve(5 + packedData.size());
    
    // Address (2 bytes)
    data.append(static_cast<char>((startAddress >> 8) & 0xFF));
    data.append(static_cast<char>(startAddress & 0xFF));
    
    // Quantity (2 bytes)
    data.append(static_cast<char>((quantity >> 8) & 0xFF));
    data.append(static_cast<char>(quantity & 0xFF));
    
    // Byte Count (1 byte)
    data.append(static_cast<char>(expectedBytes));
    
    // Data
    data.append(packedData);

    return PduBuildResult::ok(Pdu(FunctionCode::WriteMultipleCoils, data));
}

PduBuildResult ModbusPduBuilder::buildWriteMultipleRegisters(int startAddress, int quantity, const QByteArray& packedData) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Invalid start address"));
    }
    if (quantity <= 0 || quantity > 123) { // Modbus limit for TCP/RTU
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Invalid quantity for 0x10"));
    }

    if (packedData.size() != quantity * 2) {
        return PduBuildResult::fail(QCoreApplication::translate("ModbusPduBuilder", "Data length mismatch for 0x10"));
    }

    QByteArray data;
    data.reserve(5 + packedData.size());
    
    // Address (2 bytes)
    data.append(static_cast<char>((startAddress >> 8) & 0xFF));
    data.append(static_cast<char>(startAddress & 0xFF));
    
    // Quantity (2 bytes)
    data.append(static_cast<char>((quantity >> 8) & 0xFF));
    data.append(static_cast<char>(quantity & 0xFF));
    
    // Byte Count (1 byte)
    data.append(static_cast<char>(quantity * 2));
    
    // Data
    data.append(packedData);

    return PduBuildResult::ok(Pdu(FunctionCode::WriteMultipleRegisters, data));
}

} // namespace modbus::base
