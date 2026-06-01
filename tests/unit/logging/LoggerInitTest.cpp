#include <gtest/gtest.h>

#include "../../../core/logging/Logger.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

TEST(LoggerInit, EmptyLogDirectoryReturnsError)
{
    QString errorMessage;
    EXPECT_FALSE(logging::Init(QString(), &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}

TEST(LoggerInit, FilePathAsDirectoryReturnsError)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString blockedPath = QDir(sandbox.path()).filePath(QStringLiteral("not-a-directory"));
    QFile blockedFile(blockedPath);
    ASSERT_TRUE(blockedFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
    blockedFile.close();

    QString errorMessage;
    EXPECT_FALSE(logging::Init(blockedPath, &errorMessage));
    EXPECT_FALSE(errorMessage.isEmpty());
}
