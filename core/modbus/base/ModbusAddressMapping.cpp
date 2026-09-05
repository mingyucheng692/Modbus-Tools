/**
 * @file ModbusAddressMapping.cpp
 * @brief Implementation of Modbus address mapping and translation.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "modbus/base/ModbusAddressMapping.h"
#include <limits>

namespace modbus::address {

namespace {

struct ParsedInputNumber {
    qint64 value = 0;
    bool isHex = false;
    bool isValid = false;
    int digitCount = 0;
    bool startsWithZero = false;
};

ParsedInputNumber parseRawNumber(const QString& inputAddress) {
    ParsedInputNumber result;
    const QString s = inputAddress.trimmed();
    if (s.isEmpty()) {
        return result;
    }

    const bool has0x = s.startsWith(QLatin1String("0x"), Qt::CaseInsensitive);
    const bool hasH = s.endsWith(QLatin1Char('h'), Qt::CaseInsensitive) ||
                      s.endsWith(QLatin1Char('H'), Qt::CaseInsensitive);

    if (has0x && hasH) {
        // Ambiguous format: mutually exclusive
        return result;
    }

    bool parseOk = false;
    if (has0x) {
        result.value = s.mid(2).toLongLong(&parseOk, 16);
        result.isHex = true;
    } else if (hasH) {
        result.value = s.left(s.length() - 1).toLongLong(&parseOk, 16);
        result.isHex = true;
    } else {
        result.value = s.toLongLong(&parseOk, 10);
        result.isHex = false;
        result.digitCount = s.length();
        result.startsWithZero = s.startsWith(QLatin1Char('0'));
    }

    result.isValid = parseOk;
    return result;
}

} // namespace

AddressMappingResult toPduAddress(const QString& inputAddress, AddressBase base) {
    AddressMappingResult res;
    const QString trimmed = inputAddress.trimmed();
    if (trimmed.isEmpty()) {
        res.errorMessage = QStringLiteral("Address cannot be empty");
        return res;
    }

    const auto parsed = parseRawNumber(trimmed);
    if (!parsed.isValid) {
        res.errorMessage = QStringLiteral("Invalid address format");
        return res;
    }

    if (base == AddressBase::Offset0Based) {
        if (parsed.value < 0 || parsed.value > 65535) {
            res.errorMessage = QStringLiteral("Address out of range (0 - 65535)");
            return res;
        }
        res.pduAddress = static_cast<uint16_t>(parsed.value);
        res.isValid = true;
        return res;
    }

    // PlcAddress1Based
    if (parsed.isHex) {
        if (parsed.value < 1) {
            res.errorMessage = QStringLiteral("Address in 1-based mode must be >= 1");
            return res;
        }
        if (parsed.value > 65536) {
            res.errorMessage = QStringLiteral("Address out of range (1 - 65536)");
            return res;
        }
        res.pduAddress = static_cast<uint16_t>(parsed.value - 1);
        res.isValid = true;
        return res;
    }

    // Decimal handling: check Modicon notation
    const qint64 val = parsed.value;

    // 6-digit Modicon Extended Notation (e.g. 400001 - 465536)
    if (parsed.digitCount == 6) {
        if (val >= 400001 && val <= 465536) {
            res.pduAddress = static_cast<uint16_t>(val - 400001);
            res.suggestedFunctionCode = 0x03; // Read Holding Registers
            res.isValid = true;
            return res;
        }
        if (val >= 300001 && val <= 365536) {
            res.pduAddress = static_cast<uint16_t>(val - 300001);
            res.suggestedFunctionCode = 0x04; // Read Input Registers
            res.isValid = true;
            return res;
        }
        if (val >= 100001 && val <= 165536) {
            res.pduAddress = static_cast<uint16_t>(val - 100001);
            res.suggestedFunctionCode = 0x02; // Read Discrete Inputs
            res.isValid = true;
            return res;
        }
        if (parsed.startsWithZero && val >= 1 && val <= 65536) {
            res.pduAddress = static_cast<uint16_t>(val - 1);
            res.suggestedFunctionCode = 0x01; // Read Coils
            res.isValid = true;
            return res;
        }
    }

    // 5-digit Modicon Standard Notation (e.g. 40001 - 49999)
    if (parsed.digitCount == 5) {
        if (val >= 40001 && val <= 49999) {
            res.pduAddress = static_cast<uint16_t>(val - 40001);
            res.suggestedFunctionCode = 0x03; // Read Holding Registers
            res.isValid = true;
            return res;
        }
        if (val >= 30001 && val <= 39999) {
            res.pduAddress = static_cast<uint16_t>(val - 30001);
            res.suggestedFunctionCode = 0x04; // Read Input Registers
            res.isValid = true;
            return res;
        }
        if (val >= 10001 && val <= 19999) {
            res.pduAddress = static_cast<uint16_t>(val - 10001);
            res.suggestedFunctionCode = 0x02; // Read Discrete Inputs
            res.isValid = true;
            return res;
        }
        if (parsed.startsWithZero && val >= 1 && val <= 9999) {
            res.pduAddress = static_cast<uint16_t>(val - 1);
            res.suggestedFunctionCode = 0x01; // Read Coils
            res.isValid = true;
            return res;
        }
    }

    // Standard 1-based Decimal (1 ~ 65536)
    if (val < 1) {
        res.errorMessage = QStringLiteral("Address in 1-based mode must be >= 1");
        return res;
    }
    if (val > 65536) {
        res.errorMessage = QStringLiteral("Address out of range (1 - 65536)");
        return res;
    }

    res.pduAddress = static_cast<uint16_t>(val - 1);
    res.isValid = true;
    return res;
}

QString toDisplayAddress(uint16_t pduAddress, AddressBase base, bool asHex) {
    if (base == AddressBase::Offset0Based) {
        if (asHex) {
            return QStringLiteral("0x%1").arg(QString::number(pduAddress, 16).toUpper().rightJustified(4, QLatin1Char('0')));
        }
        return QString::number(pduAddress);
    }

    // PlcAddress1Based
    const uint32_t displayVal = static_cast<uint32_t>(pduAddress) + 1;
    if (asHex) {
        return QStringLiteral("0x%1").arg(QString::number(displayVal, 16).toUpper().rightJustified(4, QLatin1Char('0')));
    }
    return QString::number(displayVal);
}

} // namespace modbus::address
