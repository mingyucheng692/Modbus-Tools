// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QMetaType>
#include <optional>

// Role: Protocol-level register value interpretation type.
// Hosting note: physically placed under modbus/base (not analyzer/) so the
// base-layer register decoder can consume it without inverting the core
// dependency direction (base must not depend on analyzer). The
// modbus::analyzer namespace is retained for source compatibility with the
// existing analyzer/UI call sites.
namespace modbus::analyzer {

enum class RegisterDataType {
    UInt16,       ///< 16-bit Unsigned Integer (default 1 word)
    Int16,        ///< 16-bit Signed Integer (1 word)
    Float32,      ///< 32-bit Single-Precision Float (2 words, IEEE 754)
    Int32,        ///< 32-bit Signed Double-Word (2 words)
    UInt32,       ///< 32-bit Unsigned Double-Word (2 words)
    Float64       ///< 64-bit Double-Precision Float (4 words, IEEE 754)
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
    // Defensive path for out-of-range casts (e.g. restored settings): render
    // empty instead of silently mislabeling the value as UInt16.
    return QString();
}

inline std::optional<RegisterDataType> stringToRegisterDataType(const QString& str)
{
    const QString lower = str.trimmed().toLower();
    if (lower == QStringLiteral("uint16") || lower == QStringLiteral("unsigned")) {
        return RegisterDataType::UInt16;
    }
    if (lower == QStringLiteral("int16") || lower == QStringLiteral("signed")) {
        return RegisterDataType::Int16;
    }
    if (lower == QStringLiteral("float32") || lower == QStringLiteral("float") || lower == QStringLiteral("real")) {
        return RegisterDataType::Float32;
    }
    if (lower == QStringLiteral("int32") || lower == QStringLiteral("dint")) {
        return RegisterDataType::Int32;
    }
    if (lower == QStringLiteral("uint32") || lower == QStringLiteral("udint")) {
        return RegisterDataType::UInt32;
    }
    if (lower == QStringLiteral("float64") || lower == QStringLiteral("double")) {
        return RegisterDataType::Float64;
    }
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

} // namespace modbus::analyzer

Q_DECLARE_METATYPE(modbus::analyzer::RegisterDataType)
