/**
 * @file ValueFormatter.h
 * @brief Static utility for formatting Modbus data values.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 */

#pragma once

#include <QString>
#include <QVariant>
#include <QCoreApplication>
#include "analyzer/AnalyzerCommon.h"

namespace modbus::analyzer::value_formatter {

Q_DECLARE_TR_FUNCTIONS(modbus::analyzer::value_formatter)

/**
 * @brief 格式化为十进制字符串（考虑有无符号模式）。
 */
QString formatDecimalValue(const QVariant& value, NumberDisplayMode mode);

/**
 * @brief 提取用于显示的数值（考虑符号位转换）。
 */
double numericValueForDisplay(const QVariant& value, NumberDisplayMode mode, bool* ok = nullptr);

/**
 * @brief 格式化为缩放后的物理值。
 */
QString formatScaledValue(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode);

/**
 * @brief 生成包含原始值、缩放系数和结果的 Tooltip。
 */
QString buildDescriptionTooltip(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode);

/**
 * @brief 格式化为十六进制字符串（带 0x 前缀，保持位宽）。
 */
QString formatHexValue(const QByteArray& rawBytes, const QString& fallbackHex);

/**
 * @brief 格式化为二进制字符串（分组显示，保持位宽）。
 */
QString formatBinaryValue(const QByteArray& rawBytes, const QString& fallbackBinary);

} // namespace modbus::analyzer::value_formatter
