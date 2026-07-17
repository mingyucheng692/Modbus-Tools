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
 * @brief Namespace of free functions for CSV and JSON data handling.
 *
 * Qt JSON dependencies are kept in the .cpp.
 */
namespace exporter {

/**
 * @brief Save metadata to a JSON file.
 */
bool saveMetadataJson(const QString& filePath,
                      const QString& startAddress,
                      const QString& displayMode,
                      const QMap<uint16_t, DataMetadata>& metadata,
                      QString* error = nullptr);

/**
 * @brief Load metadata from a JSON file.
 */
ImportResult loadMetadataJson(const QString& filePath);

/**
 * @brief Write a chunk of CSV lines to a file.
 * @param firstChunk If true, truncates file and writes BOM. Otherwise appends.
 */
bool writeCsvChunk(const QString& filePath,
                   const QStringList& lines,
                   bool firstChunk,
                   QString* error = nullptr);

/**
 * @brief Escape a string for CSV format.
 */
QString escapeCsvValue(const QString& value);

} // namespace exporter

} // namespace modbus::analyzer
