// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QMetaType>
#include <optional>

// RegisterDataType and its helpers are protocol-level types hosted under
// modbus/base (see RegisterDataType.h) to keep the base -> analyzer
// dependency direction clean.
#include "../modbus/base/RegisterDataType.h"

namespace modbus::analyzer {

enum class NumberDisplayMode {
    Unsigned,
    Signed
};

// Role: Metadata descriptor for parsed register data (custom type override, scale, description).
// Thread-safety: Value type; thread-safe for distinct instances or read-only access.
// Ownership: Value object passed by copy or const reference.
struct DataMetadata {
    std::optional<RegisterDataType> customType = std::nullopt; // Optional custom type override per register
    double scale = 1.0;
    QString description;
};

} // namespace modbus::analyzer

Q_DECLARE_METATYPE(modbus::analyzer::NumberDisplayMode)
Q_DECLARE_METATYPE(modbus::analyzer::DataMetadata)
