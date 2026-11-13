// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "infra/platform/PathResolver.h"

#include <QCoreApplication>
#include <QDir>

namespace {

constexpr auto kLogsDirectoryName = "logs";
constexpr auto kUpdateStagingDirectoryName = "update";

QString normalizeApplicationDirPath(const QString& applicationDirPath)
{
    QString resolved = applicationDirPath.trimmed();
    if (resolved.isEmpty() && QCoreApplication::instance() != nullptr) {
        resolved = QCoreApplication::applicationDirPath().trimmed();
    }
    if (resolved.isEmpty()) {
        resolved = QDir::tempPath();
    }
    return QDir::cleanPath(QDir(resolved).absolutePath());
}

QString resolveAbsoluteSubdir(const QString& baseDir, const char* subdirName)
{
    const QString normalizedBase = normalizeApplicationDirPath(baseDir);
    return QDir::cleanPath(QDir(normalizedBase).absoluteFilePath(QString::fromLatin1(subdirName)));
}

} // namespace

namespace infra::platform {

PathResolver::PathResolver()
    : PathResolver(QString())
{
}

PathResolver::PathResolver(const QString& applicationDirPath)
    : applicationDirPath_(normalizeApplicationDirPath(applicationDirPath))
{
}

QString PathResolver::resolveLogDir() const
{
    return resolveAbsoluteSubdir(applicationDirPath_, kLogsDirectoryName);
}

QString PathResolver::resolveConfigDir() const
{
    return applicationDirPath_;
}

QString PathResolver::resolveUpdateStagingDir() const
{
    return resolveAbsoluteSubdir(applicationDirPath_, kUpdateStagingDirectoryName);
}

} // namespace infra::platform
