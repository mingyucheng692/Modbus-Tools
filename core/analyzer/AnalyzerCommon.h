/**
 * @file AnalyzerCommon.h
 * @brief Common types for Frame Analyzer.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <QMetaType>
#include <optional>

namespace modbus::analyzer {

/**
 * @brief Multi-register data types for industrial decoding.
 */
enum class RegisterDataType {
    UInt16,       ///< 16-bit Unsigned Integer (default 1 word)
    Int16,        ///< 16-bit Signed Integer (1 word)
    Float32,      ///< 32-bit Single-Precision Float (2 words, IEEE 754)
    Int32,        ///< 32-bit Signed Double-Word (2 words)
    UInt32,       ///< 32-bit Unsigned Double-Word (2 words)
    Float64       ///< 64-bit Double-Precision Float (4 words, IEEE 754)
};

/**
 * @brief Numeric display mode for parsed values (backward compatibility).
 */
enum class NumberDisplayMode {
    Unsigned,
    Signed
};

/**
 * @brief Metadata for parsed register data.
 */
struct DataMetadata {
    std::optional<RegisterDataType> customType = std::nullopt; ///< Optional custom type override per register
    double scale = 1.0;
    QString description;
};

} // namespace modbus::analyzer

Q_DECLARE_METATYPE(modbus::analyzer::RegisterDataType)
Q_DECLARE_METATYPE(modbus::analyzer::NumberDisplayMode)
Q_DECLARE_METATYPE(modbus::analyzer::DataMetadata)
