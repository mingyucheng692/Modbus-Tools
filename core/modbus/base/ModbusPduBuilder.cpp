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
#include <cstring>

namespace modbus::base {

namespace {

void appendBigEndianUInt16(QByteArray& data, qsizetype offset, uint16_t value)
{
    uint16_t encoded = qToBigEndian(value);
    std::memcpy(data.data() + offset, &encoded, sizeof(encoded));
}

} // namespace

std::optional<Pdu> ModbusPduBuilder::buildReadRequest(FunctionCode fc, int startAddress, int quantity,
                                                       QString* errorOut) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Invalid start address");
        return std::nullopt;
    }
    if (quantity <= 0 || quantity > 2000) {
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Invalid quantity");
        return std::nullopt;
    }

    QByteArray data;
    data.resize(4);
    appendBigEndianUInt16(data, 0, static_cast<uint16_t>(startAddress));
    appendBigEndianUInt16(data, 2, static_cast<uint16_t>(quantity));

    return Pdu(fc, data);
}

std::optional<Pdu> ModbusPduBuilder::buildWriteSingleCoil(int startAddress, bool value,
                                                           QString* errorOut) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Invalid start address");
        return std::nullopt;
    }

    QByteArray data;
    data.resize(4);
    appendBigEndianUInt16(data, 0, static_cast<uint16_t>(startAddress));
    uint16_t coilValue = value ? 0xFF00 : 0x0000;
    appendBigEndianUInt16(data, 2, coilValue);

    return Pdu(FunctionCode::WriteSingleCoil, data);
}

std::optional<Pdu> ModbusPduBuilder::buildWriteSingleRegister(int startAddress, uint16_t value,
                                                              QString* errorOut) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Invalid start address");
        return std::nullopt;
    }

    QByteArray data;
    data.resize(4);
    appendBigEndianUInt16(data, 0, static_cast<uint16_t>(startAddress));
    appendBigEndianUInt16(data, 2, value);

    return Pdu(FunctionCode::WriteSingleRegister, data);
}

std::optional<Pdu> ModbusPduBuilder::buildWriteMultipleCoils(int startAddress, int quantity,
                                                              const QByteArray& packedData,
                                                              QString* errorOut) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Invalid start address");
        return std::nullopt;
    }
    if (quantity <= 0 || quantity > 1968) { // Modbus limit
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Invalid quantity for 0x0F");
        return std::nullopt;
    }

    int expectedBytes = (quantity + 7) / 8;
    if (packedData.size() != expectedBytes) {
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Data length mismatch for 0x0F");
        return std::nullopt;
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

    return Pdu(FunctionCode::WriteMultipleCoils, data);
}

std::optional<Pdu> ModbusPduBuilder::buildWriteMultipleRegisters(int startAddress, int quantity,
                                                                  const QByteArray& packedData,
                                                                  QString* errorOut) {
    if (startAddress < 0 || startAddress > 0xFFFF) {
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Invalid start address");
        return std::nullopt;
    }
    if (quantity <= 0 || quantity > 123) { // Modbus limit for TCP/RTU
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Invalid quantity for 0x10");
        return std::nullopt;
    }

    if (packedData.size() != quantity * 2) {
        if (errorOut) *errorOut = QCoreApplication::translate("ModbusPduBuilder", "Data length mismatch for 0x10");
        return std::nullopt;
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

    return Pdu(FunctionCode::WriteMultipleRegisters, data);
}

} // namespace modbus::base
