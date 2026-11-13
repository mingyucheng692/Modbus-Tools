// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "infra/platform/win/Win32ProcessRunner.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <vector>
#include <windows.h>
#include <shellapi.h>

namespace {

struct LaunchPaths {
    std::wstring nativeExecutable;
    std::wstring nativeWorkingDirectory;
};

LaunchPaths prepareLaunchPaths(const QString& executablePath)
{
    const QString nativeExe = QDir::toNativeSeparators(executablePath);
    const QString nativeWorkDir = QDir::toNativeSeparators(QFileInfo(executablePath).absolutePath());
    return {
        nativeExe.toStdWString(),
        nativeWorkDir.toStdWString(),
    };
}

} // namespace

namespace infra::platform {

QString Win32ProcessRunner::quoteProcessArgument(const QString& argument)
{
    if (argument.isEmpty()) {
        return QStringLiteral("\"\"");
    }

    const bool needsQuoting = std::any_of(argument.cbegin(), argument.cend(), [](QChar ch) {
        const ushort code = ch.unicode();
        return code == u' ' || code == u'\t' || code == u'\n' || code == u'\v' || code == u'"';
    });
    if (!needsQuoting) {
        return argument;
    }

    QString quoted;
    quoted.reserve(argument.size() + 4);
    quoted.append(QLatin1Char('"'));

    int backslashCount = 0;
    for (const QChar ch : argument) {
        if (ch == QLatin1Char('\\')) {
            ++backslashCount;
            continue;
        }
        if (ch == QLatin1Char('"')) {
            quoted.append(QString(backslashCount * 2 + 1, QLatin1Char('\\')));
            quoted.append(QLatin1Char('"'));
            backslashCount = 0;
            continue;
        }
        if (backslashCount > 0) {
            quoted.append(QString(backslashCount, QLatin1Char('\\')));
            backslashCount = 0;
        }
        quoted.append(ch);
    }

    if (backslashCount > 0) {
        quoted.append(QString(backslashCount * 2, QLatin1Char('\\')));
    }
    quoted.append(QLatin1Char('"'));
    return quoted;
}

QString Win32ProcessRunner::buildProcessArguments(const QStringList& arguments)
{
    QStringList quotedArguments;
    quotedArguments.reserve(arguments.size());
    for (const QString& argument : arguments) {
        quotedArguments.push_back(quoteProcessArgument(argument));
    }
    return quotedArguments.join(QLatin1Char(' '));
}

bool Win32ProcessRunner::supportsElevatedLaunch() const noexcept
{
    return true;
}

bool Win32ProcessRunner::startElevated(const QString& executablePath,
                                       const QStringList& arguments,
                                       QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    const LaunchPaths paths = prepareLaunchPaths(executablePath);
    const std::wstring parametersW = buildProcessArguments(arguments).toStdWString();

    SHELLEXECUTEINFOW shellExecInfo{};
    shellExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
    shellExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    shellExecInfo.lpVerb = L"runas";
    shellExecInfo.lpFile = paths.nativeExecutable.c_str();
    shellExecInfo.lpParameters = parametersW.empty() ? nullptr : parametersW.c_str();
    shellExecInfo.lpDirectory = paths.nativeWorkingDirectory.c_str();
    shellExecInfo.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&shellExecInfo)) {
        const DWORD lastError = GetLastError();
        SPDLOG_ERROR("Win32ProcessRunner: ShellExecuteExW failed with error {}",
                     static_cast<unsigned long>(lastError));
        if (errorMessage != nullptr) {
            *errorMessage = (lastError == ERROR_CANCELLED)
                ? QStringLiteral("Elevated launch was cancelled by user (ERROR_CANCELLED, error %1)")
                      .arg(static_cast<unsigned long>(lastError))
                : QStringLiteral("Failed to launch elevated updater process (error %1)")
                      .arg(static_cast<unsigned long>(lastError));
        }
        return false;
    }

    if (shellExecInfo.hProcess != nullptr) {
        CloseHandle(shellExecInfo.hProcess);
    }
    return true;
}

bool Win32ProcessRunner::startNonElevated(const QString& executablePath,
                                          const QStringList& arguments,
                                          QString* errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    const LaunchPaths paths = prepareLaunchPaths(executablePath);
    const QString quotedExecutable = quoteProcessArgument(QDir::toNativeSeparators(executablePath));
    const QString quotedArgs = buildProcessArguments(arguments);
    const QString commandLine = quotedArgs.isEmpty()
        ? quotedExecutable
        : QStringLiteral("%1 %2").arg(quotedExecutable, quotedArgs);
    const std::wstring commandLineW = commandLine.toStdWString();

    // Build a mutable copy for CreateProcessW (it modifies the buffer).
    std::vector<wchar_t> mutableCmdLine(commandLineW.begin(), commandLineW.end());
    mutableCmdLine.push_back(L'\0');

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(STARTUPINFOW);
    PROCESS_INFORMATION processInfo{};

    const BOOL success = CreateProcessW(
        paths.nativeExecutable.c_str(),
        mutableCmdLine.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        paths.nativeWorkingDirectory.c_str(),
        &startupInfo,
        &processInfo
    );

    if (!success) {
        const DWORD lastError = GetLastError();
        SPDLOG_ERROR("Win32ProcessRunner: CreateProcessW failed with error {}",
                     static_cast<unsigned long>(lastError));
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to launch updater process (error %1)")
                                .arg(static_cast<unsigned long>(lastError));
        }
        return false;
    }

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    return true;
}

} // namespace infra::platform
