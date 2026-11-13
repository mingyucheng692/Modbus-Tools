// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ValueFormatter.h"
#include "modbus/base/RegisterValueDecoder.h"
#include <QRegularExpression>
#include <cmath>

namespace modbus::analyzer {

QString value_formatter::formatDecimalValue(const QVariant& value, NumberDisplayMode mode)
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

double value_formatter::numericValueForDisplay(const QVariant& value, NumberDisplayMode mode, bool* ok)
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

QString value_formatter::formatScaledValue(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode)
{
    bool ok = false;
    const double raw = numericValueForDisplay(value, mode, &ok);
    if (!ok) return QStringLiteral("-");
    // QVariant doubles can carry NaN/±Inf (e.g. IEEE-754 payload decoded as
    // float); intercept before the scale multiply so the output never
    // renders as "nan"/"inf" — matching the rawBytes path's decoding guard.
    if (std::isnan(raw) || std::isinf(raw)) return QStringLiteral("-");
    return QString::number(raw * meta.scale, 'g', 12);
}

QString value_formatter::formatScaledValue(
    QByteArrayView rawBytes,
    const DataMetadata& meta,
    RegisterDataType type,
    modbus::base::RegisterOrder order)
{
    const auto decoded = modbus::codec::decodeRegister(rawBytes, type, order);
    if (!decoded.isValid) {
        return decoded.displayText;
    }
    return modbus::codec::formatEngineeringValue(decoded, meta.scale, type);
}

QString value_formatter::buildDescriptionTooltip(const QVariant& value, const DataMetadata& meta, NumberDisplayMode mode)
{
    QStringList lines;
    if (!meta.description.trimmed().isEmpty()) {
        lines << QCoreApplication::translate("modbus::analyzer::value_formatter", "Description: %1")
                     .arg(meta.description.trimmed());
    }
    bool ok = false;
    const double raw = numericValueForDisplay(value, mode, &ok);
    if (ok && !std::isnan(raw) && !std::isinf(raw)) {
        const double scaled = raw * meta.scale;
        lines << QCoreApplication::translate("modbus::analyzer::value_formatter", "Raw: %1")
                     .arg(QString::number(raw, 'g', 12));
        lines << QCoreApplication::translate("modbus::analyzer::value_formatter", "Scale: %1")
                     .arg(QString::number(meta.scale, 'g', 12));
        lines << QCoreApplication::translate("modbus::analyzer::value_formatter", "Scaled: %1")
                     .arg(QString::number(scaled, 'g', 12));
    }
    return lines.join('\n');
}

QString value_formatter::buildDescriptionTooltip(
    QByteArrayView rawBytes,
    const DataMetadata& meta,
    RegisterDataType type,
    modbus::base::RegisterOrder order)
{
    QStringList lines;
    if (!meta.description.trimmed().isEmpty()) {
        lines << QCoreApplication::translate("modbus::analyzer::value_formatter", "Description: %1")
                     .arg(meta.description.trimmed());
    }
    const auto decoded = modbus::codec::decodeRegister(rawBytes, type, order);
    if (decoded.isValid) {
        lines << QCoreApplication::translate("modbus::analyzer::value_formatter", "Raw: %1")
                     .arg(decoded.displayText);
        lines << QCoreApplication::translate("modbus::analyzer::value_formatter", "Scale: %1")
                     .arg(QString::number(meta.scale, 'g', 12));
        if (decoded.isNanOrInf) {
            lines << QCoreApplication::translate("modbus::analyzer::value_formatter", "Scaled: %1")
                         .arg(decoded.displayText);
        } else {
            lines << QCoreApplication::translate("modbus::analyzer::value_formatter", "Scaled: %1")
                         .arg(modbus::codec::formatEngineeringValue(decoded, meta.scale, type));
        }
    }
    return lines.join('\n');
}

QString value_formatter::formatHexValue(const QByteArray& rawBytes, const QString& fallbackHex)
{
    if (rawBytes.isEmpty()) return fallbackHex;
    
    bool ok = false;
    const QString hexString = QString(rawBytes.toHex()).toUpper();
    const uint32_t rawValue = hexString.toUInt(&ok, 16);
    if (!ok) return fallbackHex;

    const int digits = hexString.size();
    uint32_t mask = 0xFFFFFFFFu;
    int bitWidth = qMax(1, digits * 4);
    if (bitWidth < 32) mask = (1u << bitWidth) - 1u;

    const uint32_t masked = rawValue & mask;
    const QString rawHex = QString("%1").arg(masked, digits, 16, QChar('0')).toUpper();
    return QStringLiteral("0x%1").arg(rawHex);
}

QString value_formatter::formatBinaryValue(const QByteArray& rawBytes, const QString& fallbackBinary)
{
    if (rawBytes.isEmpty()) return fallbackBinary;

    bool ok = false;
    const QString hexString = QString(rawBytes.toHex()).toUpper();
    const uint32_t rawValue = hexString.toUInt(&ok, 16);
    if (!ok) return fallbackBinary;

    int bitWidth = hexString.size() * 4;
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

namespace modbus::codec {

// Moved here from base/RegisterValueDecoder.cpp (its only consumers are the
// rawBytes formatting paths above); see ValueFormatter.h for the rationale.
QString formatEngineeringValue(
    const DecodedRegisterValue& decoded,
    double scale,
    modbus::analyzer::RegisterDataType type
) {
    if (!decoded.isValid) {
        // Short-circuit: if NaN or Inf, do NOT multiply by scale, preserve clean displayText
        return decoded.displayText;
    }

    if (scale == 1.0) {
        return decoded.displayText;
    }

    const double scaledVal = decoded.numericValue * scale;
    if (std::isnan(scaledVal)) {
        return QStringLiteral("NaN");
    }
    if (std::isinf(scaledVal)) {
        return (scaledVal > 0) ? QStringLiteral("+Inf") : QStringLiteral("-Inf");
    }

    switch (type) {
    case modbus::analyzer::RegisterDataType::Float32:
        return formatFloatScientific(scaledVal, 7);
    case modbus::analyzer::RegisterDataType::Float64:
        return formatFloatScientific(scaledVal, 15);
    default:
        // Integer types scaled to floating values
        return formatFloatScientific(scaledVal, 12);
    }
}

} // namespace modbus::codec
