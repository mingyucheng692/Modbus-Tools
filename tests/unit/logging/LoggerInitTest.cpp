// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "../../../core/Config.h"
#include "../../../infra/logging/Logger.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <memory>
#include <spdlog/spdlog.h>

namespace {

struct SpdlogFallbackGuard {
    ~SpdlogFallbackGuard()
    {
        if (!spdlog::default_logger()) {
            spdlog::set_default_logger(std::make_shared<spdlog::logger>("test_fallback"));
        }
    }
};

} // namespace

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

TEST(LoggerInit, FailedInitLeavesAppRunnableInDegradedMode)
{
    // Contract for the portable-only deployment: when the exe directory is
    // not writable, main() shows a warning and KEEPS RUNNING (see main.cpp).
    // A failed Init returns BEFORE touching the spdlog registry, so whatever
    // default logger is installed (the built-in stdout one in a fresh
    // process) keeps accepting SPDLOG_* macro calls without crashing.
    // spdlog::shutdown() itself is NOT exercised here: it resets the default
    // logger, and re-arming it afterwards would be observable by unrelated
    // tests sharing this process.
    QString errorMessage;
    EXPECT_FALSE(logging::Init(QString(), &errorMessage));

    EXPECT_NO_FATAL_FAILURE({ SPDLOG_INFO("degraded-mode line after failed Init"); });
}

TEST(LoggerInit, WritableDirCreatesPidFilePrunesOldLogsAndShutdownFlushes)
{
    SpdlogFallbackGuard fallbackGuard;

    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());
    QDir logDir(sandbox.path());

    const int seedCount = static_cast<int>(config::Logging::kMaxRotatedFiles) + 2;
    for (int i = 0; i < seedCount; ++i) {
        const QString oldPath = logDir.filePath(
            QStringLiteral("modbus-tools_20200101-00000%1_999%1.log").arg(i));
        QFile oldFile(oldPath);
        ASSERT_TRUE(oldFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
        oldFile.write("stale\n");
        oldFile.setFileTime(QDateTime::fromSecsSinceEpoch(1577836800 + i), QFileDevice::FileModificationTime);
        oldFile.close();
    }

    QString errorMessage = QStringLiteral("stale");
    ASSERT_TRUE(logging::Init(sandbox.path(), &errorMessage)) << errorMessage.toStdString();
    EXPECT_TRUE(errorMessage.isEmpty());

    qWarning("logger-qt-forward-marker");
    logging::Shutdown();

    EXPECT_EQ(spdlog::default_logger(), nullptr);

    logDir.setNameFilters({QStringLiteral("modbus-tools_*.log*")});
    logDir.setFilter(QDir::Files | QDir::NoSymLinks);
    const QStringList entries = logDir.entryList();
    EXPECT_LE(entries.size(), static_cast<int>(config::Logging::kMaxRotatedFiles));

    const QRegularExpression pidPattern(
        QStringLiteral("^modbus-tools_\\d{8}-\\d{6}Z_%1\\.log$").arg(QCoreApplication::applicationPid()));
    QString currentLogFile;
    for (const QString& entry : entries) {
        if (pidPattern.match(entry).hasMatch()) {
            currentLogFile = logDir.filePath(entry);
            break;
        }
    }
    ASSERT_FALSE(currentLogFile.isEmpty());

    QFile currentFile(currentLogFile);
    ASSERT_TRUE(currentFile.open(QIODevice::ReadOnly));
    const QByteArray content = currentFile.readAll();
    EXPECT_TRUE(content.contains("logger-qt-forward-marker"));
}
