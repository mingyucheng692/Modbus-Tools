/**
 * @file ValueFormatter.h
 * @brief Static utility for formatting Modbus data values.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 */

#pragma once

#include <QString>
#include <QVariant>
#include <QByteArray>
#include <QByteArrayView>
#include <QCoreApplication>
#include "analyzer/AnalyzerCommon.h"
#include "modbus/base/ModbusTypes.h"

namespace modbus::analyzer::value_formatter {

inline QString tr(const char* sourceText, const char* disambiguation = nullptr, int n = -1)
{
    return QCoreApplication::translate("modbus::analyzer::value_formatter", sourceText, disambiguation, n);
}

/**
 * @brief 格式化为十进制字符串（考虑有无符号模式）。
 */
QString formatDecimalValue(const QVariant& value, NumberDisplayMode mode);

/**
 * @brief 提取用于显示的数值（考虑符号位转换）。
 */
double numericValueForDisplay(const QVariant& value, NumberDisplayMode mode, bool* ok = nullptr);

/**
 * @brief 格式化为缩放后的物理值（16位兼容重载）。
 */
QString formatScaledValue(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode);

/**
 * @brief 格式化为缩放后的物理值（支持多寄存器强类型与字序重排）。
 */
QString formatScaledValue(QByteArrayView rawBytes, const DataMetadata& meta, RegisterDataType type, modbus::base::RegisterOrder order);

/**
 * @brief 生成包含原始值、缩放系数和结果的 Tooltip（16位兼容重载）。
 */
QString buildDescriptionTooltip(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode);

/**
 * @brief 生成包含原始值、缩放系数和结果的 Tooltip（多寄存器与字序支持）。
 */
QString buildDescriptionTooltip(QByteArrayView rawBytes, const DataMetadata& meta, RegisterDataType type, modbus::base::RegisterOrder order);

/**
 * @brief 格式化为十六进制字符串（带 0x 前缀，保持位宽）。
 */
QString formatHexValue(const QByteArray& rawBytes, const QString& fallbackHex);

/**
 * @brief 格式化为二进制字符串（分组显示，保持位宽）。
 */
QString formatBinaryValue(const QByteArray& rawBytes, const QString& fallbackBinary);

} // namespace modbus::analyzer::value_formatter
