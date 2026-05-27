/**
 * @file Win32ProcessRunner.cpp
 * @brief Implements the Windows elevated process launcher.
 */

#include "infra/platform/win/Win32ProcessRunner.h"

#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <spdlog/spdlog.h>
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

} // namespace infra::platform
