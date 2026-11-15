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

value_formatter::FrameDecodeEvaluation value_formatter::evaluateFrameDecodeCapability(
    const ::modbus::parser::ParseResult& pr,
    bool isTx
) {
    using ::modbus::base::FunctionCode;
    FrameDecodeEvaluation eval;

    if (!pr.isValid && pr.error.isEmpty() && pr.rawFrame.isEmpty()) {
        eval.capability = FrameDecodeCapability::NoDataPayload;
        eval.capabilityHint = QCoreApplication::translate("ValueFormatter", "No Modbus frame payload");
        return eval;
    }

    if (pr.isException || static_cast<int>(pr.functionCode) >= 0x80) {
        eval.capability = FrameDecodeCapability::NoDataPayload;
        eval.capabilityHint = QCoreApplication::translate("ValueFormatter", "Exception response contains no register data payload");
        return eval;
    }

    // Read Requests (0x01..0x04 TX)
    if (isTx && (pr.functionCode == FunctionCode::ReadCoils ||
                 pr.functionCode == FunctionCode::ReadDiscreteInputs ||
                 pr.functionCode == FunctionCode::ReadHoldingRegisters ||
                 pr.functionCode == FunctionCode::ReadInputRegisters)) {
        eval.capability = FrameDecodeCapability::NoDataPayload;
        eval.capabilityHint = QCoreApplication::translate("ValueFormatter", "Read request frame contains no register data payload");
        return eval;
    }

    // Write Echo Confirmations (0x0F, 0x10 RX)
    if (!isTx && (pr.functionCode == FunctionCode::WriteMultipleCoils ||
                  pr.functionCode == FunctionCode::WriteMultipleRegisters)) {
        eval.capability = FrameDecodeCapability::NoDataPayload;
        eval.capabilityHint = QCoreApplication::translate("ValueFormatter", "Write confirmation echo contains no register data payload");
        return eval;
    }

    // Write Single Register (0x06 TX & RX)
    if (pr.functionCode == FunctionCode::WriteSingleRegister) {
        eval.capability = FrameDecodeCapability::SingleRegister;
        eval.enableDecodeCombo = true;
        eval.enableOrderCombo = false;
        eval.allowMultiWordTypes = false;
        eval.capabilityHint = QCoreApplication::translate("ValueFormatter", "Multi-word byte ordering not applicable to 16-bit register");
        if (!pr.dataItems.isEmpty() && pr.dataItems[0].rawBytes.size() >= 2) {
            eval.registerBytes = pr.dataItems[0].rawBytes.left(2);
            eval.startAddress = pr.dataItems[0].address;
            eval.registerCount = 1;
        } else if (pr.pduData.size() == 5) {
            eval.registerBytes = pr.pduData.mid(3, 2);
            eval.startAddress = (static_cast<uint8_t>(pr.pduData[1]) << 8) | static_cast<uint8_t>(pr.pduData[2]);
            eval.registerCount = 1;
        }
        return eval;
    }

    // Discrete Bits: 0x01/0x02 RX, 0x05 TX/RX, 0x0F TX
    if (pr.functionCode == FunctionCode::ReadCoils ||
        pr.functionCode == FunctionCode::ReadDiscreteInputs ||
        pr.functionCode == FunctionCode::WriteSingleCoil ||
        (pr.functionCode == FunctionCode::WriteMultipleCoils && isTx)) {
        eval.capability = FrameDecodeCapability::DiscreteBits;
        eval.enableDecodeCombo = false;
        eval.enableOrderCombo = false;
        eval.allowMultiWordTypes = false;
        eval.capabilityHint = QCoreApplication::translate("ValueFormatter", "Discrete coil/bit operation");
        return eval;
    }

    // Full Registers: 0x03/0x04 RX, 0x10 TX
    if (((pr.functionCode == FunctionCode::ReadHoldingRegisters ||
          pr.functionCode == FunctionCode::ReadInputRegisters) && !isTx) ||
        (pr.functionCode == FunctionCode::WriteMultipleRegisters && isTx)) {
        eval.capability = FrameDecodeCapability::FullRegisters;
        eval.enableDecodeCombo = true;
        eval.enableOrderCombo = true;
        eval.allowMultiWordTypes = true;
        eval.capabilityHint.clear();

        for (const auto& item : pr.dataItems) {
            if (item.rawBytes.size() >= 2) {
                eval.registerBytes.append(item.rawBytes.left(2));
            }
        }
        eval.registerCount = eval.registerBytes.size() / 2;
        if (!pr.dataItems.isEmpty()) {
            eval.startAddress = pr.dataItems[0].address;
        }
        return eval;
    }

    eval.capability = FrameDecodeCapability::NoDataPayload;
    eval.capabilityHint = QCoreApplication::translate("ValueFormatter", "No Modbus register payload");
    return eval;
}

QString value_formatter::formatDecodedSummary(
    const ::modbus::parser::ParseResult& pr,
    const FrameDecodeEvaluation& eval,
    RegisterDataType type,
    modbus::base::RegisterOrder order,
    const QMap<uint16_t, DataMetadata>& metadata
) {
    if (eval.capability == FrameDecodeCapability::DiscreteBits) {
        if (pr.dataItems.isEmpty()) {
            return QCoreApplication::translate("ValueFormatter", "No discrete bit data available");
        }
        QStringList lines;
        lines.reserve(pr.dataItems.size());
        for (int i = 0; i < pr.dataItems.size(); ++i) {
            const auto& item = pr.dataItems[i];
            const bool isOn = item.value.toBool();
            const QString state = isOn ? QStringLiteral("ON") : QStringLiteral("OFF");
            if (metadata.contains(item.address) && !metadata.value(item.address).description.isEmpty()) {
                lines.append(QStringLiteral("[+%1] Coil %2 (%3): %4")
                                 .arg(i, 2, 10, QLatin1Char('0'))
                                 .arg(item.address, 4, 10, QLatin1Char('0'))
                                 .arg(metadata.value(item.address).description)
                                 .arg(state));
            } else {
                lines.append(QStringLiteral("[+%1] Coil %2: %3")
                                 .arg(i, 2, 10, QLatin1Char('0'))
                                 .arg(item.address, 4, 10, QLatin1Char('0'))
                                 .arg(state));
            }
        }
        return lines.join(QLatin1Char('\n'));
    }

    if (eval.capability == FrameDecodeCapability::SingleRegister) {
        if (eval.registerBytes.size() < 2) {
            return QCoreApplication::translate("ValueFormatter", "<Incomplete register data>");
        }
        const RegisterDataType actType = (type == RegisterDataType::Int16) ? RegisterDataType::Int16 : RegisterDataType::UInt16;
        const QByteArrayView slice(eval.registerBytes.constData(), 2);
        const uint16_t addr = eval.startAddress;
        if (metadata.contains(addr)) {
            const auto& meta = metadata.value(addr);
            const QString valStr = formatScaledValue(slice, meta, actType, order);
            if (!meta.description.isEmpty()) {
                return QStringLiteral("[+00] Addr %1 (%2): %3")
                    .arg(addr, 4, 10, QLatin1Char('0'))
                    .arg(meta.description)
                    .arg(valStr);
            }
            return QStringLiteral("[+00] Addr %1: %2")
                .arg(addr, 4, 10, QLatin1Char('0'))
                .arg(valStr);
        }
        const auto decoded = modbus::codec::decodeRegister(slice, actType, order);
        return QStringLiteral("[+00] Addr %1: %2")
            .arg(addr, 4, 10, QLatin1Char('0'))
            .arg(decoded.displayText);
    }

    if (eval.capability == FrameDecodeCapability::FullRegisters) {
        int reqBytes = 2;
        switch (type) {
        case RegisterDataType::UInt16:
        case RegisterDataType::Int16:
            reqBytes = 2;
            break;
        case RegisterDataType::UInt32:
        case RegisterDataType::Int32:
        case RegisterDataType::Float32:
            reqBytes = 4;
            break;
        case RegisterDataType::Float64:
            reqBytes = 8;
            break;
        }

        if (eval.registerBytes.size() < reqBytes) {
            return QCoreApplication::translate("ValueFormatter",
                "<Incomplete: requires %1 registers (%2 bytes), available: %3 (%4 bytes)>")
                .arg(reqBytes / 2)
                .arg(reqBytes)
                .arg(eval.registerCount)
                .arg(eval.registerBytes.size());
        }

        QStringList lines;
        const int totalBytes = eval.registerBytes.size();
        int offset = 0;
        int regIndex = 0;
        while (offset + reqBytes <= totalBytes) {
            const QByteArrayView slice(eval.registerBytes.constData() + offset, reqBytes);
            const uint16_t addr = eval.startAddress + regIndex;
            QString line;
            if (metadata.contains(addr)) {
                const auto& meta = metadata.value(addr);
                const QString valStr = formatScaledValue(slice, meta, type, order);
                if (!meta.description.isEmpty()) {
                    line = QStringLiteral("[+%1] Addr %2 (%3): %4")
                        .arg(regIndex, 2, 10, QLatin1Char('0'))
                        .arg(addr, 4, 10, QLatin1Char('0'))
                        .arg(meta.description)
                        .arg(valStr);
                } else {
                    line = QStringLiteral("[+%1] Addr %2: %3")
                        .arg(regIndex, 2, 10, QLatin1Char('0'))
                        .arg(addr, 4, 10, QLatin1Char('0'))
                        .arg(valStr);
                }
            } else {
                const auto decoded = modbus::codec::decodeRegister(slice, type, order);
                line = QStringLiteral("[+%1] Addr %2: %3")
                    .arg(regIndex, 2, 10, QLatin1Char('0'))
                    .arg(addr, 4, 10, QLatin1Char('0'))
                    .arg(decoded.displayText);
            }
            lines.append(line);
            offset += reqBytes;
            regIndex += (reqBytes / 2);
        }

        if (offset < totalBytes) {
            const int leftoverBytes = totalBytes - offset;
            const int leftoverRegs = leftoverBytes / 2;
            lines.append(QStringLiteral("[+%1] Leftover: <Incomplete: %2 registers (%3 bytes) available, %4 required>")
                             .arg(regIndex, 2, 10, QLatin1Char('0'))
                             .arg(leftoverRegs)
                             .arg(leftoverBytes)
                             .arg(reqBytes / 2));
        }
        return lines.join(QLatin1Char('\n'));
    }

    return QString();
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
