#include <gtest/gtest.h>

#include "../../../infra/logging/Logger.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <spdlog/spdlog.h>

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
