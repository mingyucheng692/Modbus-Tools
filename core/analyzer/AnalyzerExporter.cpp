/**
 * @file AnalyzerExporter.cpp
 * @brief Implementation of AnalyzerExporter utility.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "AnalyzerExporter.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>

namespace modbus::analyzer {

bool AnalyzerExporter::saveMetadataJson(const QString& filePath,
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

ImportResult AnalyzerExporter::loadMetadataJson(const QString& filePath) {
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
        result.metadata.insert(static_cast<uint16_t>(addressValue), meta);
    }

    result.success = true;
    return result;
}

bool AnalyzerExporter::writeCsvChunk(const QString& filePath,
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

    return true;
}

QString AnalyzerExporter::escapeCsvValue(const QString& value) {
    QString escaped = value;
    escaped.replace(QLatin1Char('"'), QLatin1String("\"\""));
    return QStringLiteral("\"%1\"").arg(escaped);
}

} // namespace modbus::analyzer
