/**
 * @file ModbusDataHelper.cpp
 * @brief Implementation of ModbusDataHelper.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ModbusDataHelper.h"
#include <QRegularExpression>
#include <QStringList>

namespace modbus::base {

QByteArray ModbusDataHelper::parseHex(const QString& input) {
    QString cleaned = input;
    cleaned.remove(QRegularExpression("[^0-9A-Fa-f]"));
    if (cleaned.isEmpty()) return QByteArray();
    if (cleaned.size() % 2 == 1) cleaned.prepend("0");
    return QByteArray::fromHex(cleaned.toLatin1());
}

QByteArray ModbusDataHelper::parseDecimalList(const QString& input, bool& ok) {
    ok = true;
    QByteArray result;
    QStringList parts = input.split(QRegularExpression("[\\s,;]+"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        ok = false;
        return result;
    }
    for (const auto& part : parts) {
        bool partOk = false;
        uint value = part.toUInt(&partOk, 10);
        if (!partOk || value > 65535) {
            ok = false;
            return QByteArray();
        }
        // Big-endian register packing
        result.append(static_cast<char>((value >> 8) & 0xFF));
        result.append(static_cast<char>(value & 0xFF));
    }
    return result;
}

QByteArray ModbusDataHelper::parseBinary(const QString& input) {
    QString bits = input;
    bits.remove(QRegularExpression("[^0-1]"));
    if (bits.isEmpty()) return QByteArray();
    
    int bitCount = bits.size();
    int byteCount = (bitCount + 7) / 8;
    QByteArray result(byteCount, 0);
    
    // Modbus Coils: Address (N) maps to Bit (N % 8) of Byte (N / 8)
    // The first coil (base address) is the Least Significant Bit of the first byte.
    for (int i = 0; i < bitCount; ++i) {
        if (bits[i] == '1') {
            result[i / 8] = result[i / 8] | (1 << (i % 8));
        }
    }
    return result;
}

} // namespace modbus::base
