// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "analyzer/AnalyzerExporter.h"
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using namespace modbus::analyzer;

TEST(AnalyzerExporterTest, WriteCsvChunk_WritesAndReportsSuccess) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    QString error;
    const bool ok = exporter::writeCsvChunk(tempDir.filePath("out.csv"),
                                            {QStringLiteral("a,b"), QStringLiteral("\"q\"")},
                                            /*firstChunk=*/true, &error);
    ASSERT_TRUE(ok) << error.toStdString();

    QFile file(tempDir.filePath("out.csv"));
    ASSERT_TRUE(file.open(QIODevice::ReadOnly));
    // The exporter opens the file in Text mode, so line endings are the
    // platform EOL (CRLF on Windows); compare EOL-agnostically.
    const QString text = QString::fromUtf8(file.readAll())
                             .replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    EXPECT_TRUE(text.contains("a,b\n"));
    EXPECT_TRUE(text.contains("\"q\"\n"));
}

TEST(AnalyzerExporterTest, EscapeCsvValue_NeutralizesFormulaInjection) {
    // CWE-1236: leading formula triggers are defused with a tick prefix.
    EXPECT_EQ(exporter::escapeCsvValue(QStringLiteral("=1+1")), QStringLiteral("\"'=1+1\""));
    EXPECT_EQ(exporter::escapeCsvValue(QStringLiteral("@SUM(A1)")), QStringLiteral("\"'@SUM(A1)\""));
    EXPECT_EQ(exporter::escapeCsvValue(QStringLiteral("-cmd")), QStringLiteral("\"'-cmd\""));
    EXPECT_EQ(exporter::escapeCsvValue(QStringLiteral("+cmd")), QStringLiteral("\"'+cmd\""));
    // Leading whitespace before a trigger is still defused.
    EXPECT_EQ(exporter::escapeCsvValue(QStringLiteral(" =cmd")), QStringLiteral("\"' =cmd\""));

    // Plain signed numbers stay numeric.
    EXPECT_EQ(exporter::escapeCsvValue(QStringLiteral("-5.2")), QStringLiteral("\"-5.2\""));
    EXPECT_EQ(exporter::escapeCsvValue(QStringLiteral("+3")), QStringLiteral("\"+3\""));

    // Normal text and quote escaping unchanged.
    EXPECT_EQ(exporter::escapeCsvValue(QStringLiteral("a\"b")), QStringLiteral("\"a\"\"b\""));
}

TEST(AnalyzerExporterTest, RoundtripWithCustomTypes) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString filePath = tempDir.filePath("test_config.json");

    QMap<uint16_t, DataMetadata> metadata;
    DataMetadata m1;
    m1.customType = RegisterDataType::Float32;
    m1.scale = 0.1;
    m1.description = "Temperature Sensor";
    metadata.insert(100, m1);

    DataMetadata m2;
    m2.customType = RegisterDataType::Float64;
    m2.scale = 1.0;
    m2.description = "Power Accumulator";
    metadata.insert(200, m2);

    DataMetadata m3; // Default type (nullopt)
    m3.customType = std::nullopt;
    m3.scale = 2.5;
    m3.description = "Speed RPM";
    metadata.insert(300, m3);

    QString error;
    bool saveOk = exporter::saveMetadataJson(filePath, "40001", "Float32", metadata, &error);
    ASSERT_TRUE(saveOk) << error.toStdString();

    ImportResult result = exporter::loadMetadataJson(filePath);
    ASSERT_TRUE(result.success) << result.error.toStdString();
    EXPECT_EQ(result.startAddress, "40001");
    EXPECT_EQ(result.displayMode, "float32");
    ASSERT_EQ(result.metadata.size(), 3);

    EXPECT_TRUE(result.metadata.contains(100));
    EXPECT_EQ(result.metadata[100].customType, RegisterDataType::Float32);
    EXPECT_DOUBLE_EQ(result.metadata[100].scale, 0.1);
    EXPECT_EQ(result.metadata[100].description, "Temperature Sensor");

    EXPECT_TRUE(result.metadata.contains(200));
    EXPECT_EQ(result.metadata[200].customType, RegisterDataType::Float64);
    EXPECT_DOUBLE_EQ(result.metadata[200].scale, 1.0);
    EXPECT_EQ(result.metadata[200].description, "Power Accumulator");

    EXPECT_TRUE(result.metadata.contains(300));
    EXPECT_FALSE(result.metadata[300].customType.has_value());
    EXPECT_DOUBLE_EQ(result.metadata[300].scale, 2.5);
    EXPECT_EQ(result.metadata[300].description, "Speed RPM");
}

TEST(AnalyzerExporterTest, BackwardCompatibilityWithoutType) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString filePath = tempDir.filePath("legacy_config.json");

    // Construct a legacy JSON without "type" field
    QJsonObject root;
    root["version"] = 1;
    root["startAddress"] = "0";
    root["displayMode"] = "signed";

    QJsonArray items;
    QJsonObject item;
    item["address"] = 10;
    item["description"] = "Legacy Item";
    item["scale"] = 1.0;
    items.append(item);
    root["items"] = items;

    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(root).toJson());
    file.close();

    ImportResult result = exporter::loadMetadataJson(filePath);
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.startAddress, "0");
    EXPECT_EQ(result.displayMode, "signed");
    ASSERT_EQ(result.metadata.size(), 1);
    EXPECT_TRUE(result.metadata.contains(10));
    EXPECT_FALSE(result.metadata[10].customType.has_value());
    EXPECT_EQ(result.metadata[10].description, "Legacy Item");
}

TEST(AnalyzerExporterTest, CsvExportAndEscape) {
    EXPECT_EQ(exporter::escapeCsvValue("normal"), "\"normal\"");
    EXPECT_EQ(exporter::escapeCsvValue("with \"quotes\""), "\"with \"\"quotes\"\"\"");

    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString csvPath = tempDir.filePath("test.csv");

    QStringList lines = {"col1,col2", "val1,val2"};
    QString error;
    bool writeOk = exporter::writeCsvChunk(csvPath, lines, true, &error);
    EXPECT_TRUE(writeOk) << error.toStdString();
}
