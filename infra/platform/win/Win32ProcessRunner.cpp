/**
 * @file Win32ProcessRunner.cpp
 * @brief Implements the Windows elevated and non-elevated process launcher.
 */

#include "infra/platform/win/Win32ProcessRunner.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <spdlog/spdlog.h>
#include <vector>
#include <windows.h>
#include <shellapi.h>

namespace {

QString quoteProcessArgument(const QString& argument)
{
    QString escaped = argument;
    escaped.replace('"', QStringLiteral("\\\""));
    if (escaped.contains(' ') || escaped.contains('\t') || escaped.contains('"')) {
        return QStringLiteral("\"%1\"").arg(escaped);
    }
    return escaped;
}

QString buildProcessArguments(const QStringList& arguments)
{
    QStringList quotedArguments;
    quotedArguments.reserve(arguments.size());
    for (const QString& argument : arguments) {
        quotedArguments.push_back(quoteProcessArgument(argument));
    }
    return quotedArguments.join(' ');
}

} // namespace

namespace infra::platform {

bool Win32ProcessRunner::supportsElevatedLaunch() const noexcept
{
    return true;
}

bool Win32ProcessRunner::startElevated(const QString& executablePath,
                                       const QStringList& arguments,
                                       QString* errorMessage)
{
    const QString nativeExecutablePath = QDir::toNativeSeparators(executablePath);
    const QString nativeWorkingDirectory = QDir::toNativeSeparators(QFileInfo(executablePath).absolutePath());
    const QString parameters = buildProcessArguments(arguments);

    SHELLEXECUTEINFOW shellExecInfo{};
    shellExecInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
    shellExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shellExecInfo.hwnd = nullptr;
    shellExecInfo.lpVerb = L"runas";

    const std::wstring executablePathW = nativeExecutablePath.toStdWString();
    const std::wstring parametersW = parameters.toStdWString();
    const std::wstring workingDirectoryW = nativeWorkingDirectory.toStdWString();
    shellExecInfo.lpFile = executablePathW.c_str();
    shellExecInfo.lpParameters = parametersW.c_str();
    shellExecInfo.lpDirectory = workingDirectoryW.c_str();
    shellExecInfo.nShow = SW_SHOWNORMAL;

    if (!ShellExecuteExW(&shellExecInfo)) {
        const DWORD lastError = GetLastError();
        spdlog::error("Win32ProcessRunner: ShellExecuteExW failed with error {}", static_cast<unsigned long>(lastError));
        if (errorMessage != nullptr) {
            *errorMessage = QStringLiteral("Failed to launch updater (Access Denied or System Error)");
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
    const QString nativeExecutablePath = QDir::toNativeSeparators(executablePath);
    const QString nativeWorkingDirectory = QDir::toNativeSeparators(QFileInfo(executablePath).absolutePath());
    const QString commandLine = QStringLiteral("\"%1\" %2")
                                    .arg(nativeExecutablePath, buildProcessArguments(arguments));

    const std::wstring commandLineW = commandLine.toStdWString();
    const std::wstring workingDirectoryW = nativeWorkingDirectory.toStdWString();

    // Build a mutable copy for CreateProcessW (it modifies the buffer).
    std::vector<wchar_t> mutableCmdLine(commandLineW.begin(), commandLineW.end());
    mutableCmdLine.push_back(L'\0');

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(STARTUPINFOW);
    PROCESS_INFORMATION processInfo{};

    const BOOL success = CreateProcessW(
        nativeExecutablePath.toStdWString().c_str(),  // lpApplicationName
        mutableCmdLine.data(),                         // lpCommandLine
        nullptr,                                       // lpProcessAttributes
        nullptr,                                       // lpThreadAttributes
        FALSE,                                         // bInheritHandles
        CREATE_NEW_CONSOLE,                            // dwCreationFlags
        nullptr,                                       // lpEnvironment
        workingDirectoryW.c_str(),                     // lpCurrentDirectory
        &startupInfo,                                  // lpStartupInfo
        &processInfo                                   // lpProcessInformation
    );

    if (!success) {
        const DWORD lastError = GetLastError();
        spdlog::error("Win32ProcessRunner: CreateProcessW failed with error {}", static_cast<unsigned long>(lastError));
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
