// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "common/FileExporter.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextStream>

namespace ui::common {

TEST(FileExporterTest, ExportLinesToFile_WritesUtf8Cleanly) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());
    const QString filePath = tempDir.filePath(QStringLiteral("sample.csv"));

    QString error;
    ASSERT_TRUE(exportLinesToFile(
        filePath,
        {QStringLiteral("col1,col2"), QStringLiteral("hello,world")},
        &error)) << error.toStdString();
    EXPECT_TRUE(error.isEmpty());

    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    const QStringList readLines = in.readAll().split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    ASSERT_EQ(readLines.size(), 2);
    EXPECT_EQ(readLines.at(0), QStringLiteral("col1,col2"));
    EXPECT_EQ(readLines.at(1), QStringLiteral("hello,world"));
}

TEST(FileExporterTest, ExportLinesToFile_InvalidTargetReturnsFalseWithError) {
    QTemporaryDir tempDir;
    ASSERT_TRUE(tempDir.isValid());

    QString error;
    EXPECT_FALSE(exportLinesToFile(tempDir.path(), {QStringLiteral("data")}, &error));
    EXPECT_FALSE(error.isEmpty());

    error.clear();
    const QString missingParentPath = QDir(tempDir.path()).filePath(QStringLiteral("missing_dir/out.txt"));
    EXPECT_FALSE(exportLinesToFile(missingParentPath, {QStringLiteral("data")}, &error));
    EXPECT_FALSE(error.isEmpty());
}

} // namespace ui::common
