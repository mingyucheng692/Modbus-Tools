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

inline QString registerDataTypeToString(RegisterDataType type)
{
    switch (type) {
        case RegisterDataType::UInt16: return QStringLiteral("UInt16");
        case RegisterDataType::Int16: return QStringLiteral("Int16");
        case RegisterDataType::Float32: return QStringLiteral("Float32");
        case RegisterDataType::Int32: return QStringLiteral("Int32");
        case RegisterDataType::UInt32: return QStringLiteral("UInt32");
        case RegisterDataType::Float64: return QStringLiteral("Float64");
    }
    return QStringLiteral("UInt16");
}

inline std::optional<RegisterDataType> stringToRegisterDataType(const QString& str)
{
    const QString lower = str.trimmed().toLower();
    if (lower == QStringLiteral("uint16") || lower == QStringLiteral("unsigned")) return RegisterDataType::UInt16;
    if (lower == QStringLiteral("int16") || lower == QStringLiteral("signed")) return RegisterDataType::Int16;
    if (lower == QStringLiteral("float32") || lower == QStringLiteral("float") || lower == QStringLiteral("real")) return RegisterDataType::Float32;
    if (lower == QStringLiteral("int32") || lower == QStringLiteral("dint")) return RegisterDataType::Int32;
    if (lower == QStringLiteral("uint32") || lower == QStringLiteral("udint")) return RegisterDataType::UInt32;
    if (lower == QStringLiteral("float64") || lower == QStringLiteral("double")) return RegisterDataType::Float64;
    return std::nullopt;
}

inline int registerWordsCount(RegisterDataType type)
{
    switch (type) {
        case RegisterDataType::Float64:
            return 4;
        case RegisterDataType::Float32:
        case RegisterDataType::Int32:
        case RegisterDataType::UInt32:
            return 2;
        case RegisterDataType::UInt16:
        case RegisterDataType::Int16:
        default:
            return 1;
    }
}

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
