// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "ModbusDataHelper.h"
#include <QRegularExpression>
#include <QStringList>

namespace ui::common {

QByteArray data_helper::parseHex(const QString& input) {
    QString cleaned = input;
    cleaned.remove(QRegularExpression("[^0-9A-Fa-f]"));
    if (cleaned.isEmpty()) return QByteArray();
    if (cleaned.size() % 2 == 1) cleaned.prepend("0");
    return QByteArray::fromHex(cleaned.toLatin1());
}

QByteArray data_helper::parseDecimalList(const QString& input, bool& ok) {
    ok = true;
    QByteArray result;
    QStringList parts = input.split(QRegularExpression("[\\s,;]+"), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        ok = false;
        return result;
    }
    for (const auto& part : parts) {
        bool partOk = false;
        uint value = part.toUInt(&partOk, 10);
        if (!partOk || value > 65535) {
            ok = false;
            return QByteArray();
        }
        // Big-endian register packing
        result.append(static_cast<char>((value >> 8) & 0xFF));
        result.append(static_cast<char>(value & 0xFF));
    }
    return result;
}

QByteArray data_helper::parseBinary(const QString& input) {
    QString bits = input;
    bits.remove(QRegularExpression("[^0-1]"));
    if (bits.isEmpty()) return QByteArray();
    
    int bitCount = bits.size();
    int byteCount = (bitCount + 7) / 8;
    QByteArray result(byteCount, 0);
    
    // Modbus Coils: Address (N) maps to Bit (N % 8) of Byte (N / 8)
    // The first coil (base address) is the Least Significant Bit of the first byte.
    for (int i = 0; i < bitCount; ++i) {
        if (bits[i] == '1') {
            result[i / 8] = result[i / 8] | (1 << (i % 8));
        }
    }
    return result;
}

int data_helper::parseSmartInt(const QString& input, bool* ok) {
    if (ok) *ok = false;
    QString s = input.trimmed();
    if (s.isEmpty()) return 0;

    const bool has0x = s.startsWith(QLatin1String("0x"), Qt::CaseInsensitive);
    const bool hasH = s.endsWith(QLatin1Char('h'), Qt::CaseInsensitive) || 
                      s.endsWith(QLatin1Char('H'), Qt::CaseInsensitive);

    if (has0x && hasH) {
        // Ambiguous format (mutually exclusive)
        return 0;
    }

    bool parseOk = false;
    int val = 0;
    if (has0x) {
        val = s.mid(2).toInt(&parseOk, 16);
    } else if (hasH) {
        val = s.left(s.length() - 1).toInt(&parseOk, 16);
    } else {
        val = s.toInt(&parseOk, 10);
    }

    if (ok) *ok = parseOk;
    return val;
}

std::optional<QString> data_helper::convertPastedText(const QString& text, int targetRadix) noexcept {
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty() || (targetRadix != 10 && targetRadix != 16)) {
        return std::nullopt;
    }

    const QStringList parts = trimmed.split(QRegularExpression(QStringLiteral("[\\s,;]+")), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return std::nullopt;
    }

    QStringList convertedParts;
    convertedParts.reserve(parts.size());

    for (const QString& part : parts) {
        const bool has0x = part.startsWith(QLatin1String("0x"), Qt::CaseInsensitive);
        const bool hasH = part.endsWith(QLatin1Char('h'), Qt::CaseInsensitive) ||
                          part.endsWith(QLatin1Char('H'), Qt::CaseInsensitive);
        if (has0x && hasH) {
            return std::nullopt;
        }

        QString coreStr = part;
        bool isExplicitHex = false;
        if (has0x) {
            coreStr = part.mid(2);
            isExplicitHex = true;
        } else if (hasH) {
            coreStr = part.left(part.length() - 1);
            isExplicitHex = true;
        }

        coreStr = coreStr.trimmed();
        if (coreStr.isEmpty()) {
            return std::nullopt;
        }

        bool ok = false;
        uint32_t val = 0;
        if (isExplicitHex) {
            val = coreStr.toUInt(&ok, 16);
        } else {
            bool hasHexAlpha = false;
            for (const QChar& ch : coreStr) {
                if ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
                    hasHexAlpha = true;
                    break;
                }
            }
            val = hasHexAlpha ? coreStr.toUInt(&ok, 16) : coreStr.toUInt(&ok, 10);
        }

        if (!ok || val > 65535U) {
            return std::nullopt;
        }

        convertedParts.append(targetRadix == 16 ? QString::number(val, 16).toUpper()
                                                : QString::number(val, 10));
    }

    return convertedParts.join(QLatin1Char(' '));
}

QString data_helper::convertRadix(const QString& text, int fromRadix, int toRadix) noexcept {
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty() || fromRadix == toRadix ||
        (fromRadix != 10 && fromRadix != 16) || (toRadix != 10 && toRadix != 16)) {
        return trimmed;
    }

    const QStringList parts = trimmed.split(QRegularExpression(QStringLiteral("[\\s,;]+")), Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return QString();
    }

    QStringList resultParts;
    resultParts.reserve(parts.size());

    for (const QString& part : parts) {
        QString s = part;
        if (fromRadix == 16) {
            if (s.startsWith(QLatin1String("0x"), Qt::CaseInsensitive)) {
                s = s.mid(2);
            } else if (s.endsWith(QLatin1Char('h'), Qt::CaseInsensitive) ||
                       s.endsWith(QLatin1Char('H'), Qt::CaseInsensitive)) {
                s = s.left(s.length() - 1);
            }
        }
        bool ok = false;
        const uint32_t val = s.toUInt(&ok, fromRadix);
        if (!ok || val > 65535U) {
            return trimmed;
        }
        resultParts.append(toRadix == 16 ? QString::number(val, 16).toUpper()
                                         : QString::number(val, 10));
    }

    return resultParts.join(QLatin1Char(' '));
}

::common::Result<QByteArray, QString> data_helper::packCoilBits(const QString& bitStr, int expectedQuantity) noexcept {
    if (expectedQuantity <= 0) {
        return ::common::Result<QByteArray, QString>::err(
            QStringLiteral("Expected quantity must be greater than zero"));
    }

    QString cleaned = bitStr;
    cleaned.remove(QRegularExpression(QStringLiteral("[\\s,;]+")));

    for (const QChar& ch : cleaned) {
        if (ch != QLatin1Char('0') && ch != QLatin1Char('1')) {
            return ::common::Result<QByteArray, QString>::err(
                QStringLiteral("Invalid bit character: only '0' and '1' are allowed"));
        }
    }

    if (cleaned.length() != expectedQuantity) {
        return ::common::Result<QByteArray, QString>::err(
            QStringLiteral("Bit count (%1) does not match expected quantity (%2)")
                .arg(cleaned.length())
                .arg(expectedQuantity));
    }

    const int byteCount = (expectedQuantity + 7) / 8;
    QByteArray result(byteCount, 0);

    for (int i = 0; i < expectedQuantity; ++i) {
        if (cleaned[i] == QLatin1Char('1')) {
            result[i / 8] = static_cast<char>(result[i / 8] | (1 << (i % 8)));
        }
    }

    return ::common::Result<QByteArray, QString>::ok(result);
}

::common::Result<QByteArray, QString> data_helper::parseRegisterWords(
    const QString& text, int radix, int expectedQuantity) noexcept {
    if (expectedQuantity <= 0) {
        return ::common::Result<QByteArray, QString>::err(
            QStringLiteral("Expected quantity must be greater than zero"));
    }
    if (radix != 10 && radix != 16) {
        return ::common::Result<QByteArray, QString>::err(
            QStringLiteral("Unsupported radix (only 10 and 16 supported)"));
    }

    const QStringList parts = text.split(QRegularExpression(QStringLiteral("[\\s,;]+")), Qt::SkipEmptyParts);
    if (parts.size() != expectedQuantity) {
        return ::common::Result<QByteArray, QString>::err(
            QStringLiteral("Word count (%1) does not match expected quantity (%2)")
                .arg(parts.size())
                .arg(expectedQuantity));
    }

    QByteArray result;
    result.reserve(expectedQuantity * 2);

    for (const QString& part : parts) {
        QString s = part;
        if (radix == 16) {
            if (s.startsWith(QLatin1String("0x"), Qt::CaseInsensitive)) {
                s = s.mid(2);
            } else if (s.endsWith(QLatin1Char('h'), Qt::CaseInsensitive) ||
                       s.endsWith(QLatin1Char('H'), Qt::CaseInsensitive)) {
                s = s.left(s.length() - 1);
            }
        }
        bool ok = false;
        const uint32_t val = s.toUInt(&ok, radix);
        if (!ok || val > 65535U) {
            return ::common::Result<QByteArray, QString>::err(
                QStringLiteral("Invalid register value: '%1'").arg(part));
        }
        result.append(static_cast<char>((val >> 8) & 0xFF));
        result.append(static_cast<char>(val & 0xFF));
    }

    return ::common::Result<QByteArray, QString>::ok(result);
}

} // namespace ui::common
