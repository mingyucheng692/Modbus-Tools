// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <QMap>
#include <QStringList>
#include "AnalyzerCommon.h"

namespace modbus::analyzer {

// Role: Result container for metadata import operations (success status, error, and parsed metadata map).
// Thread-safety: Value type; thread-safe for distinct instances.
// Ownership: Value object returned by value.
struct ImportResult {
    bool success = false;
    QString error;
    QString startAddress;
    QString displayMode;
    QMap<uint16_t, DataMetadata> metadata;
};

// Free functions for CSV and JSON data handling; Qt JSON dependencies kept in .cpp.
namespace exporter {

bool saveMetadataJson(const QString& filePath,
                      const QString& startAddress,
                      const QString& displayMode,
                      const QMap<uint16_t, DataMetadata>& metadata,
                      QString* error = nullptr);

ImportResult loadMetadataJson(const QString& filePath);

// firstChunk: If true, truncates file and writes UTF-8 BOM; otherwise appends.
bool writeCsvChunk(const QString& filePath,
                   const QStringList& lines,
                   bool firstChunk,
                   QString* error = nullptr);

QString escapeCsvValue(const QString& value);

} // namespace exporter

} // namespace modbus::analyzer
