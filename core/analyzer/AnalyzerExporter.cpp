// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "AnalyzerExporter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>

namespace modbus::analyzer {

bool exporter::saveMetadataJson(const QString& filePath,
                                        const QString& startAddress,
                                        const QString& displayMode,
                                        const QMap<uint16_t, DataMetadata>& metadata,
                                        QString* error) {
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("startAddress"), startAddress);
    root.insert(QStringLiteral("displayMode"), displayMode);

    QJsonArray items;
    for (auto it = metadata.cbegin(); it != metadata.cend(); ++it) {
        QJsonObject item;
        item.insert(QStringLiteral("address"), static_cast<int>(it.key()));
        item.insert(QStringLiteral("description"), it.value().description);
        item.insert(QStringLiteral("scale"), it.value().scale);
        if (it.value().customType.has_value()) {
            item.insert(QStringLiteral("type"), registerDataTypeToString(*it.value().customType));
        }
        items.append(item);
    }
    root.insert(QStringLiteral("items"), items);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error) *error = file.errorString();
        return false;
    }

    const QByteArray jsonBytes = QJsonDocument(root).toJson(QJsonDocument::Indented);
    if (file.write(jsonBytes) != jsonBytes.size()) {
        if (error) *error = file.errorString();
        return false;
    }

    return true;
}

ImportResult exporter::loadMetadataJson(const QString& filePath) {
    ImportResult result;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        result.success = false;
        result.error = file.errorString();
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (doc.isNull()) {
        result.success = false;
        result.error = parseError.errorString();
        return result;
    }

    const QJsonObject root = doc.object();
    
    // Parse startAddress (string or double for backward compatibility)
    const QJsonValue addrVal = root.value(QStringLiteral("startAddress"));
    if (addrVal.isString()) {
        result.startAddress = addrVal.toString();
    } else if (addrVal.isDouble()) {
        result.startAddress = QString::number(addrVal.toInt());
    }

    // Parse displayMode
    result.displayMode = root.value(QStringLiteral("displayMode")).toString().trimmed().toLower();

    // Parse items
    const QJsonArray items = root.value(QStringLiteral("items")).toArray();
    for (const QJsonValue& value : items) {
        if (!value.isObject()) continue;
        const QJsonObject itemObj = value.toObject();
        const int addressValue = itemObj.value(QStringLiteral("address")).toInt(-1);
        if (addressValue < 0 || addressValue > 65535) continue;

        DataMetadata meta;
        meta.description = itemObj.value(QStringLiteral("description")).toString();
        meta.scale = itemObj.value(QStringLiteral("scale")).toDouble(1.0);
        if (itemObj.contains(QStringLiteral("type"))) {
            meta.customType = stringToRegisterDataType(itemObj.value(QStringLiteral("type")).toString());
        }
        result.metadata.insert(static_cast<uint16_t>(addressValue), meta);
    }

    result.success = true;
    return result;
}

bool exporter::writeCsvChunk(const QString& filePath,
                                     const QStringList& lines,
                                     bool firstChunk,
                                     QString* error) {
    QFile file(filePath);
    QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
    mode |= firstChunk ? QIODevice::Truncate : QIODevice::Append;
    
    if (!file.open(mode)) {
        if (error) *error = file.errorString();
        return false;
    }

    QTextStream stream(&file);
    if (firstChunk) {
        stream.setGenerateByteOrderMark(true);
    }
    for (const QString& line : lines) {
        stream << line << '\n';
    }
    stream.flush();
    if (stream.status() != QTextStream::Ok || file.error() != QFileDevice::NoError) {
        // Disk-full / IO failures surface only on flush; silently returning
        // true would corrupt the exported archive.
        if (error) {
            *error = file.errorString().isEmpty()
                ? QStringLiteral("CSV write failed (stream status %1)").arg(static_cast<int>(stream.status()))
                : file.errorString();
        }
        return false;
    }

    return true;
}

QString exporter::escapeCsvValue(const QString& value) {
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QLatin1String("\"\""));
    // CSV formula injection (CWE-1236): spreadsheet applications interpret
    // cells starting with these characters as formulas. Prefix a tick to
    // keep the value literal; a leading +/- followed by a digit or dot is a
    // plain number and stays untouched.
    const QString trimmed = escaped.trimmed();
    if (!trimmed.isEmpty()) {
        const QChar first = trimmed.at(0);
        const bool plainNumber = (first == QLatin1Char('-') || first == QLatin1Char('+'))
            && trimmed.size() > 1
            && (trimmed.at(1).isDigit() || trimmed.at(1) == QLatin1Char('.'));
        if (!plainNumber
            && (first == QLatin1Char('=') || first == QLatin1Char('+')
                || first == QLatin1Char('-') || first == QLatin1Char('@'))) {
            escaped.prepend(QLatin1Char('\''));
        }
    }
    return QStringLiteral("\"%1\"").arg(escaped);
}

} // namespace modbus::analyzer
