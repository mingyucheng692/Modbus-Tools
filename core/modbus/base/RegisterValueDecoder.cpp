/**
 * @file RegisterValueDecoder.cpp
 * @brief Industrial multi-register value decoding implementation.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "modbus/base/RegisterValueDecoder.h"
#include "modbus/base/ModbusByteOrder.h"
#include <cmath>
#include <cstring>
#include <limits>

namespace modbus::codec {

static_assert(std::numeric_limits<float>::is_iec559, "IEEE 754 float required");
static_assert(std::numeric_limits<double>::is_iec559, "IEEE 754 double required");

QString formatFloatScientific(double val, int sigDigits) {
    if (val == 0.0) {
        return QStringLiteral("0");
    }

    const double absVal = std::abs(val);
    const double upperThreshold = (sigDigits <= 7) ? 1e7 : 1e15;

    // Force scientific notation when too small or exceeding significant precision width
    if (absVal < 1e-4 || absVal >= upperThreshold) {
        // 'e' with (sigDigits - 1) decimal places gives exactly sigDigits significant figures
        QString s = QString::number(val, 'e', sigDigits - 1);
        const int ePos = s.indexOf(QLatin1Char('e'));
        if (ePos != -1) {
            QString mantissa = s.left(ePos);
            const QString expPart = s.mid(ePos);
            if (mantissa.contains(QLatin1Char('.'))) {
                while (mantissa.endsWith(QLatin1Char('0'))) {
                    mantissa.chop(1);
                }
                if (mantissa.endsWith(QLatin1Char('.'))) {
                    mantissa.chop(1);
                }
            }
            return mantissa + expPart;
        }
        return s;
    }

    // Normal range: use 'g' with exact significant digits (Qt 'g' automatically strips trailing zeros)
    return QString::number(val, 'g', sigDigits);
}

using modbus::base::rearrange16;
using modbus::base::rearrange32;
using modbus::base::rearrange64;

DecodedRegisterValue decodeRegister(
    QByteArrayView rawBytes,
    modbus::analyzer::RegisterDataType type,
    modbus::base::RegisterOrder order
) {
    DecodedRegisterValue res;
    const auto* p = reinterpret_cast<const uint8_t*>(rawBytes.data());
    const qsizetype size = rawBytes.size();

    switch (type) {
    case modbus::analyzer::RegisterDataType::UInt16: {
        if (size < 2) {
            res.displayText = QStringLiteral("<Incomplete>");
            return res;
        }
        const uint16_t u16 = rearrange16(p, order);
        res.numericValue = static_cast<double>(u16);
        res.isValid = true;
        res.displayText = QString::number(u16);
        return res;
    }
    case modbus::analyzer::RegisterDataType::Int16: {
        if (size < 2) {
            res.displayText = QStringLiteral("<Incomplete>");
            return res;
        }
        const uint16_t u16 = rearrange16(p, order);
        int16_t i16 = 0;
        std::memcpy(&i16, &u16, sizeof(i16));
        res.numericValue = static_cast<double>(i16);
        res.isValid = true;
        res.displayText = QString::number(i16);
        return res;
    }
    case modbus::analyzer::RegisterDataType::UInt32: {
        if (size < 4) {
            res.displayText = QStringLiteral("<Incomplete>");
            return res;
        }
        const uint32_t u32 = rearrange32(p, order);
        res.numericValue = static_cast<double>(u32);
        res.isValid = true;
        res.displayText = QString::number(u32);
        return res;
    }
    case modbus::analyzer::RegisterDataType::Int32: {
        if (size < 4) {
            res.displayText = QStringLiteral("<Incomplete>");
            return res;
        }
        const uint32_t u32 = rearrange32(p, order);
        int32_t i32 = 0;
        std::memcpy(&i32, &u32, sizeof(i32));
        res.numericValue = static_cast<double>(i32);
        res.isValid = true;
        res.displayText = QString::number(i32);
        return res;
    }
    case modbus::analyzer::RegisterDataType::Float32: {
        if (size < 4) {
            res.displayText = QStringLiteral("<Incomplete>");
            return res;
        }
        const uint32_t u32 = rearrange32(p, order);
        float f32 = 0.0f;
        std::memcpy(&f32, &u32, sizeof(f32));

        if (std::isnan(f32)) {
            res.isValid = false;
            res.isNanOrInf = true;
            res.displayText = QStringLiteral("NaN");
            return res;
        }
        if (std::isinf(f32)) {
            res.isValid = false;
            res.isNanOrInf = true;
            res.displayText = (f32 > 0) ? QStringLiteral("+Inf") : QStringLiteral("-Inf");
            return res;
        }
        if (f32 == 0.0f) {
            f32 = 0.0f; // Normalize signed zero -0.0 to +0.0
        }

        res.numericValue = static_cast<double>(f32);
        res.isValid = true;
        res.displayText = formatFloatScientific(res.numericValue, 7);
        return res;
    }
    case modbus::analyzer::RegisterDataType::Float64: {
        if (size < 8) {
            res.displayText = QStringLiteral("<Incomplete>");
            return res;
        }
        const uint64_t u64 = rearrange64(p, order);
        double f64 = 0.0;
        std::memcpy(&f64, &u64, sizeof(f64));

        if (std::isnan(f64)) {
            res.isValid = false;
            res.isNanOrInf = true;
            res.displayText = QStringLiteral("NaN");
            return res;
        }
        if (std::isinf(f64)) {
            res.isValid = false;
            res.isNanOrInf = true;
            res.displayText = (f64 > 0) ? QStringLiteral("+Inf") : QStringLiteral("-Inf");
            return res;
        }
        if (f64 == 0.0) {
            f64 = 0.0; // Normalize signed zero -0.0 to +0.0
        }

        res.numericValue = f64;
        res.isValid = true;
        res.displayText = formatFloatScientific(res.numericValue, 15);
        return res;
    }
    }

    return res;
}

QString formatEngineeringValue(
    const DecodedRegisterValue& decoded,
    double scale,
    modbus::analyzer::RegisterDataType type
) {
    if (!decoded.isValid) {
        // Short-circuit: if NaN or Inf, do NOT multiply by scale, preserve clean displayText
        return decoded.displayText;
    }

    if (scale == 1.0) {
        return decoded.displayText;
    }

    const double scaledVal = decoded.numericValue * scale;
    if (std::isnan(scaledVal)) {
        return QStringLiteral("NaN");
    }
    if (std::isinf(scaledVal)) {
        return (scaledVal > 0) ? QStringLiteral("+Inf") : QStringLiteral("-Inf");
    }

    switch (type) {
    case modbus::analyzer::RegisterDataType::Float32:
        return formatFloatScientific(scaledVal, 7);
    case modbus::analyzer::RegisterDataType::Float64:
        return formatFloatScientific(scaledVal, 15);
    default:
        // Integer types scaled to floating values
        return formatFloatScientific(scaledVal, 12);
    }
}

} // namespace modbus::codec
