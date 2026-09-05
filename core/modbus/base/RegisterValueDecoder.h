/**
 * @file RegisterValueDecoder.h
 * @brief Industrial multi-register value decoding with IEEE 754 precision and byte-order handling.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <QByteArrayView>
#include "analyzer/AnalyzerCommon.h"
#include "modbus/base/ModbusTypes.h"

namespace modbus::codec {

/**
 * @brief Decoded multi-register value representation.
 */
struct DecodedRegisterValue {
    double numericValue = 0.0;    ///< Numeric value extracted from raw registers
    bool isValid = false;         ///< Whether decoding was successful and valid
    bool isNanOrInf = false;      ///< True if value is IEEE 754 NaN or Inf
    QString displayText;          ///< Display representation (formatted number, NaN, +Inf, -Inf, or error)
};

/**
 * @brief Format a double into scientific notation or compact decimal with exact significant figures.
 * 
 * Rules:
 * - If val == 0.0: returns "0".
 * - If 0 < |val| < 1e-4: forced scientific notation (e.g. 1.234567e-05).
 * - If |val| >= 1e7 (for sigDigits=7) or >= 1e15 (for sigDigits=15): forced scientific notation.
 * - Otherwise: standard compact decimal (e.g. 123.4567).
 * - Exponent uses lowercase 'e'; trailing redundant zeros are stripped.
 * 
 * @param val The floating-point value.
 * @param sigDigits Maximum number of significant digits (7 for Float32, 15 for Float64).
 * @return Formatted string.
 */
[[nodiscard]] QString formatFloatScientific(double val, int sigDigits);

/**
 * @brief Decode raw register bytes according to target data type and byte/word ordering.
 * 
 * Conforms to strict IEEE 754 specifications:
 * - 0 UB type punning via std::memcpy.
 * - Full 4-order permutations for 32-bit (ABCD, CDAB, BADC, DCBA) and 64-bit.
 * - Defense against NaN / +-Inf / Subnormal, and -0.0 normalization.
 * - Underflow protection: returns isValid=false with "<Incomplete>" if byte buffer is too short.
 * 
 * @param rawBytes Raw big-endian bytes from Modbus frame.
 * @param type Target RegisterDataType (UInt16, Int16, Float32, Int32, UInt32, Float64).
 * @param order Register byte/word order.
 * @return DecodedRegisterValue.
 */
[[nodiscard]] DecodedRegisterValue decodeRegister(
    QByteArrayView rawBytes,
    modbus::analyzer::RegisterDataType type,
    modbus::base::RegisterOrder order
);

/**
 * @brief Format decoded register value multiplied by scale.
 * 
 * Guarantees that if decoded.isNanOrInf is true, multiplication is bypassed to avoid NaN pollution.
 * 
 * @param decoded The decoded register value.
 * @param scale The physical scale factor.
 * @param type The register data type.
 * @return Formatted engineering display text.
 */
[[nodiscard]] QString formatEngineeringValue(
    const DecodedRegisterValue& decoded,
    double scale,
    modbus::analyzer::RegisterDataType type = modbus::analyzer::RegisterDataType::Float32
);

} // namespace modbus::codec
