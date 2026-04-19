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

namespace modbus::analyzer {

/**
 * @brief Numeric display mode for parsed values.
 */
enum class NumberDisplayMode {
    Unsigned,
    Signed
};

/**
 * @brief Metadata for parsed register data.
 */
struct DataMetadata {
    double scale = 1.0;
    QString description;
};

} // namespace modbus::analyzer

Q_DECLARE_METATYPE(modbus::analyzer::NumberDisplayMode)
Q_DECLARE_METATYPE(modbus::analyzer::DataMetadata)
