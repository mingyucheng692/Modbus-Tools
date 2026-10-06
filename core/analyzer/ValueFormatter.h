// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QVariant>
#include <QByteArray>
#include <QByteArrayView>
#include <QCoreApplication>
#include "analyzer/AnalyzerCommon.h"
#include "modbus/base/ModbusTypes.h"

namespace modbus::analyzer::value_formatter {

// Role: Stateless formatting utilities converting raw and decoded Modbus register values to human-readable strings.
// Thread-safety: Thread-safe (reentrant pure functions).
// Ownership: None (namespace of free functions).

inline QString tr(const char* sourceText, const char* disambiguation = nullptr, int n = -1)
{
    return QCoreApplication::translate("modbus::analyzer::value_formatter", sourceText, disambiguation, n);
}

QString formatDecimalValue(const QVariant& value, NumberDisplayMode mode);

double numericValueForDisplay(const QVariant& value, NumberDisplayMode mode, bool* ok = nullptr);

QString formatScaledValue(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode);

QString formatScaledValue(QByteArrayView rawBytes, const DataMetadata& meta, RegisterDataType type, modbus::base::RegisterOrder order);

QString buildDescriptionTooltip(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode);

QString buildDescriptionTooltip(QByteArrayView rawBytes, const DataMetadata& meta, RegisterDataType type, modbus::base::RegisterOrder order);

QString formatHexValue(const QByteArray& rawBytes, const QString& fallbackHex);

QString formatBinaryValue(const QByteArray& rawBytes, const QString& fallbackBinary);

} // namespace modbus::analyzer::value_formatter
