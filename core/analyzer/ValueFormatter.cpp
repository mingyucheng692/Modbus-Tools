/**
 * @file ValueFormatter.cpp
 * @brief Implementation of ValueFormatter.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 */

#include "ValueFormatter.h"
#include <QRegularExpression>

namespace modbus::analyzer {

QString ValueFormatter::formatDecimalValue(const QVariant& value, NumberDisplayMode mode)
{
    if (!value.isValid()) return QStringLiteral("-");
    if (value.typeId() == QMetaType::Bool) return value.toBool() ? QStringLiteral("1") : QStringLiteral("0");
    
    if (value.typeId() == QMetaType::UShort || value.typeId() == QMetaType::UInt || value.typeId() == QMetaType::Int) {
        const uint16_t uVal = static_cast<uint16_t>(value.toUInt());
        if (mode == NumberDisplayMode::Signed) {
            return QString::number(static_cast<int16_t>(uVal));
        }
        return QString::number(uVal);
    }
    return value.toString();
}

double ValueFormatter::numericValueForDisplay(const QVariant& value, NumberDisplayMode mode, bool* ok)
{
    if (ok) *ok = false;
    if (!value.isValid() || value.typeId() == QMetaType::Bool) return 0.0;

    if (value.typeId() == QMetaType::UShort || value.typeId() == QMetaType::UInt || value.typeId() == QMetaType::Int) {
        const uint16_t uVal = static_cast<uint16_t>(value.toUInt());
        if (ok) *ok = true;
        if (mode == NumberDisplayMode::Signed) {
            return static_cast<double>(static_cast<int16_t>(uVal));
        }
        return static_cast<double>(uVal);
    }
    bool localOk = false;
    const double parsed = value.toDouble(&localOk);
    if (ok) *ok = localOk;
    return localOk ? parsed : 0.0;
}

QString ValueFormatter::formatScaledValue(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode)
{
    bool ok = false;
    const double raw = numericValueForDisplay(value, mode, &ok);
    if (!ok) return QStringLiteral("-");
    return QString::number(raw * meta.scale, 'g', 12);
}

QString ValueFormatter::buildDescriptionTooltip(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode)
{
    QStringList lines;
    if (!meta.description.trimmed().isEmpty()) {
        lines << tr("Description: %1").arg(meta.description.trimmed());
    }
    bool ok = false;
    const double raw = numericValueForDisplay(value, mode, &ok);
    if (ok) {
        const double scaled = raw * meta.scale;
        lines << tr("Raw: %1").arg(QString::number(raw, 'g', 12));
        lines << tr("Scale: %1").arg(QString::number(meta.scale, 'g', 12));
        lines << tr("Scaled: %1").arg(QString::number(scaled, 'g', 12));
    }
    return lines.join('\n');
}

QString ValueFormatter::formatHexValue(const QByteArray& rawBytes, const QString& fallbackHex)
{
    QString normalized = QString(rawBytes.toHex()).toUpper();
    if (normalized.isEmpty()) normalized = fallbackHex;
    normalized.remove(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]")));
    if (normalized.isEmpty()) return fallbackHex;

    const int digits = normalized.size();
    bool ok = false;
    const uint32_t rawValue = normalized.toUInt(&ok, 16);
    if (!ok) return fallbackHex;

    uint32_t mask = 0xFFFFFFFFu;
    int bitWidth = qMax(1, digits * 4);
    if (bitWidth < 32) mask = (1u << bitWidth) - 1u;

    const uint32_t masked = rawValue & mask;
    const QString rawHex = QString("%1").arg(masked, digits, 16, QChar('0')).toUpper();
    return QStringLiteral("0x%1").arg(rawHex);
}

QString ValueFormatter::formatBinaryValue(const QByteArray& rawBytes, const QString& fallbackBinary)
{
    bool ok = false;
    uint32_t rawValue = 0;
    int bitWidth = 0;
    if (!rawBytes.isEmpty()) {
        QString normalized = QString(rawBytes.toHex()).toUpper();
        normalized.remove(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]")));
        if (normalized.isEmpty()) return fallbackBinary;
        rawValue = normalized.toUInt(&ok, 16);
        bitWidth = normalized.size() * 4;
    }

    if (!ok) {
        QString bin = fallbackBinary;
        bin.remove(QRegularExpression(QStringLiteral("[^0-1]")));
        if (bin.isEmpty()) return fallbackBinary;
        rawValue = bin.toUInt(&ok, 2);
        bitWidth = bin.size();
    }

    if (!ok) return fallbackBinary;

    uint32_t mask = 0xFFFFFFFFu;
    if (bitWidth < 32) mask = (1u << bitWidth) - 1u;
    
    const uint32_t masked = rawValue & mask;
    auto groupBits = [](const QString& bits) {
        QString grouped = bits;
        for (int k = grouped.size() - 4; k > 0; k -= 4) grouped.insert(k, ' ');
        return grouped;
    };

    return groupBits(QString("%1").arg(masked, bitWidth, 2, QChar('0')));
}

} // namespace modbus::analyzer
