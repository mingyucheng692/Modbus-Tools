/**
 * @file AnalyzerExporter.h
 * @brief Utility class for exporting/importing analyzer data.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <QMap>
#include <QStringList>
#include <QScopedPointer>
#include "AnalyzerCommon.h"

namespace modbus::analyzer {

/**
 * @brief Result of a metadata import operation.
 */
struct ImportResult {
    bool success = false;
    QString error;
    QString startAddress;
    QString displayMode;
    QMap<uint16_t, DataMetadata> metadata;
};

/**
 * @brief Utility for CSV and JSON data handling.
 * 
 * Follows PIMPL to hide implementation details and Qt JSON dependencies from headers.
 */
class AnalyzerExporter {
public:
    /**
     * @brief Save metadata to a JSON file.
     */
    static bool saveMetadataJson(const QString& filePath,
                                 const QString& startAddress,
                                 const QString& displayMode,
                                 const QMap<uint16_t, DataMetadata>& metadata,
                                 QString* error = nullptr);

    /**
     * @brief Load metadata from a JSON file.
     */
    static ImportResult loadMetadataJson(const QString& filePath);

    /**
     * @brief Write a chunk of CSV lines to a file.
     * @param firstChunk If true, truncates file and writes BOM. Otherwise appends.
     */
    static bool writeCsvChunk(const QString& filePath,
                              const QStringList& lines,
                              bool firstChunk,
                              QString* error = nullptr);

    /**
     * @brief Escape a string for CSV format.
     */
    static QString escapeCsvValue(const QString& value);

private:
    AnalyzerExporter() = delete;
};

} // namespace modbus::analyzer
