// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "infra/platform/win/Win32ProcessRunner.h"

#include <QStringList>
#include <gtest/gtest.h>
#include <memory>
#include <windows.h>
#include <shellapi.h>

namespace {

QStringList parseWithCommandLineToArgvW(const QString& commandLine)
{
    int argc = 0;
    const std::wstring wideCmd = commandLine.toStdWString();
    LPWSTR* rawArgv = CommandLineToArgvW(wideCmd.c_str(), &argc);
    if (rawArgv == nullptr) {
        return {};
    }

    auto deleter = [](LPWSTR* ptr) {
        if (ptr != nullptr) {
            LocalFree(ptr);
        }
    };
    const std::unique_ptr<LPWSTR[], decltype(deleter)> guard(rawArgv, deleter);

    QStringList parsed;
    parsed.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        parsed.push_back(QString::fromWCharArray(guard[i]));
    }
    return parsed;
}

} // namespace

TEST(Win32ProcessRunnerTest, QuoteProcessArgumentRoundTripsThroughCommandLineToArgvW)
{
    const QStringList cases{
        QStringLiteral("--restart"),
        QStringLiteral(""),
        QStringLiteral(R"(C:\Tools\updater.exe)"),
        QStringLiteral(R"(C:\Program Files\Modbus Tools\)"),
        QStringLiteral(R"(say "hello world")"),
        QStringLiteral(R"(nested \"quote\" and \slash\\)"),
        QStringLiteral(R"(path with spaces\\)"),
        QStringLiteral("  leading and trailing  "),
        QStringLiteral("tab\tseparated"),
        QStringLiteral("multi\nline\varg"),
    };

    const QString joined = infra::platform::Win32ProcessRunner::buildProcessArguments(cases);
    const QStringList parsed = parseWithCommandLineToArgvW(QStringLiteral("updater.exe %1").arg(joined));

    ASSERT_EQ(parsed.size(), cases.size() + 1);
    for (int i = 0; i < cases.size(); ++i) {
        EXPECT_EQ(parsed[i + 1], cases[i]) << "Mismatch at argument index " << i;
    }
}

TEST(Win32ProcessRunnerTest, EmptyExpectedVersionPreservesSubsequentArguments)
{
    const QStringList args{
        QStringLiteral("--expected-version"),
        QString(),
        QStringLiteral("--launcher-pid"),
        QStringLiteral("4242"),
        QStringLiteral("--restart"),
    };

    EXPECT_EQ(infra::platform::Win32ProcessRunner::quoteProcessArgument(QString()),
              QStringLiteral("\"\""));

    const QString built = infra::platform::Win32ProcessRunner::buildProcessArguments(args);
    const QStringList parsed = parseWithCommandLineToArgvW(QStringLiteral("updater.exe %1").arg(built));

    ASSERT_EQ(parsed.size(), 6);
    EXPECT_EQ(parsed[1], QStringLiteral("--expected-version"));
    EXPECT_TRUE(parsed[2].isEmpty());
    EXPECT_EQ(parsed[3], QStringLiteral("--launcher-pid"));
    EXPECT_EQ(parsed[4], QStringLiteral("4242"));
    EXPECT_EQ(parsed[5], QStringLiteral("--restart"));
}

TEST(Win32ProcessRunnerTest, MissingExecutableIncludesErrorCodeAndClearsPriorMessage)
{
    infra::platform::Win32ProcessRunner runner;
    const QString missingPath = QStringLiteral("C:/ModbusTools_NonExistent_Dir/missing_updater.exe");

    QString nonElevatedError = QStringLiteral("stale error");
    EXPECT_FALSE(runner.startNonElevated(missingPath, {QStringLiteral("--version")}, &nonElevatedError));
    EXPECT_FALSE(nonElevatedError.contains(QStringLiteral("stale error")));
    EXPECT_TRUE(nonElevatedError.contains(QStringLiteral("error ")));

    QString elevatedError = QStringLiteral("stale error");
    EXPECT_FALSE(runner.startElevated(missingPath, {QStringLiteral("--version")}, &elevatedError));
    EXPECT_FALSE(elevatedError.contains(QStringLiteral("stale error")));
    EXPECT_TRUE(elevatedError.contains(QStringLiteral("error ")));
}
