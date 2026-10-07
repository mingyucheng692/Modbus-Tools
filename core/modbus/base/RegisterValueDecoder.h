// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QByteArrayView>
#include "analyzer/AnalyzerCommon.h"
#include "modbus/base/ModbusTypes.h"

namespace modbus::codec {

struct DecodedRegisterValue {
    double numericValue = 0.0;
    bool isValid = false;
    bool isNanOrInf = false;
    QString displayText;
};

// Formats a floating-point value into scientific notation or compact decimal.
// Rules:
// - If val == 0.0: returns "0".
// - If 0 < |val| < 1e-4: forced scientific notation (e.g. 1.234567e-05).
// - If |val| >= 1e7 (sigDigits=7) or >= 1e15 (sigDigits=15): forced scientific notation.
// - Otherwise: standard compact decimal (e.g. 123.4567).
// - Exponent uses lowercase 'e'; trailing redundant zeros are stripped.
[[nodiscard]] QString formatFloatScientific(double val, int sigDigits);

// Decodes raw register bytes according to target data type and byte/word ordering.
// Conforms to strict IEEE 754 specifications:
// - 0 UB type punning via std::memcpy.
// - Full 4-order permutations for 32-bit (ABCD, CDAB, BADC, DCBA) and 64-bit.
// - Defense against NaN / +-Inf / Subnormal, and -0.0 normalization.
// - Underflow protection: returns isValid=false with "<Incomplete>" if byte buffer is too short.
[[nodiscard]] DecodedRegisterValue decodeRegister(
    QByteArrayView rawBytes,
    modbus::analyzer::RegisterDataType type,
    modbus::base::RegisterOrder order
);

// Formats decoded register value multiplied by scale.
// Guarantees that if decoded.isNanOrInf is true, multiplication is bypassed to avoid NaN pollution.
[[nodiscard]] QString formatEngineeringValue(
    const DecodedRegisterValue& decoded,
    double scale,
    modbus::analyzer::RegisterDataType type = modbus::analyzer::RegisterDataType::Float32
);

} // namespace modbus::codec

