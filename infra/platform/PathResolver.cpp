/**
 * @file PathResolver.cpp
 * @brief Implements writable path resolution for installed and portable deployments.
 */

#include "infra/platform/PathResolver.h"

#include "infra/platform/QtStandardPlatformPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <spdlog/spdlog.h>

namespace {

constexpr auto kLogsDirectoryName = "logs";
constexpr auto kProbeTemplate = ".path-resolver-write-test-XXXXXX";
constexpr auto kDefaultApplicationName = "Modbus-Tools";

QString normalizedPath(const QString& path)
{
    return QDir::cleanPath(path);
}

} // namespace

namespace infra::platform {

PathResolver::PathResolver()
    : PathResolver(std::make_shared<QtStandardPlatformPaths>(),
                   currentApplicationDirPath(),
                   currentApplicationName())
{
}

PathResolver::PathResolver(std::shared_ptr<const IPlatformPaths> platformPaths,
                           QString applicationDirPath,
                           QString applicationName)
    : platformPaths_(std::move(platformPaths)),
      applicationDirPath_(normalizedPath(applicationDirPath)),
      applicationName_(applicationName.isEmpty() ? QString::fromLatin1(kDefaultApplicationName)
                                                 : std::move(applicationName))
{
}

PathResolver& PathResolver::instance()
{
    static PathResolver resolver;
    return resolver;
}

QString PathResolver::resolveLogDir() const
{
    const QString appLogDir = QDir(applicationDirPath_).filePath(QString::fromLatin1(kLogsDirectoryName));
    const QString standardLogDir = QDir(platformPaths_->appDataLocation()).filePath(QString::fromLatin1(kLogsDirectoryName));
    return resolveWritableDir(QStringLiteral("log directory"),
                              appLogDir,
                              standardLogDir,
                              resolveScopedTempDir() + QStringLiteral("/") + QString::fromLatin1(kLogsDirectoryName));
}

QString PathResolver::resolveConfigDir() const
{
    const QString standardConfigDir = platformPaths_->appConfigLocation();
    return resolveWritableDir(QStringLiteral("config directory"),
                              applicationDirPath_,
                              standardConfigDir,
                              resolveScopedTempDir());
}

QString PathResolver::resolveTempDir() const
{
    return resolveWritableDir(QStringLiteral("temporary directory"),
                              resolveScopedTempDir(),
                              applicationDirPath_,
                              applicationDirPath_);
}

QString PathResolver::resolveScopedTempDir() const
{
    return QDir(platformPaths_->tempLocation()).filePath(applicationName_);
}

QString PathResolver::resolveWritableDir(const QString& purpose,
                                         const QString& preferredDir,
                                         const QString& secondaryDir,
                                         const QString& fallbackDir) const
{
    const QString normalizedPreferredDir = normalizedPath(preferredDir);
    if (!normalizedPreferredDir.isEmpty() && isWritableDirectory(normalizedPreferredDir)) {
        return normalizedPreferredDir;
    }

    if (!normalizedPreferredDir.isEmpty()) {
        spdlog::warn("PathResolver: {} is not writable, falling back from {}",
                     purpose.toStdString(),
                     normalizedPreferredDir.toStdString());
    }

    const QString normalizedSecondaryDir = normalizedPath(secondaryDir);
    if (!normalizedSecondaryDir.isEmpty() && normalizedSecondaryDir != normalizedPreferredDir &&
        isWritableDirectory(normalizedSecondaryDir)) {
        return normalizedSecondaryDir;
    }

    if (!normalizedSecondaryDir.isEmpty() && normalizedSecondaryDir != normalizedPreferredDir) {
        spdlog::warn("PathResolver: {} standard fallback is not writable, falling back from {}",
                     purpose.toStdString(),
                     normalizedSecondaryDir.toStdString());
    }

    const QString normalizedFallbackDir = normalizedPath(fallbackDir);
    if (!normalizedFallbackDir.isEmpty() && isWritableDirectory(normalizedFallbackDir)) {
        return normalizedFallbackDir;
    }

    if (!normalizedFallbackDir.isEmpty()) {
        spdlog::warn("PathResolver: {} fallback is not writable, using best-effort {}",
                     purpose.toStdString(),
                     normalizedFallbackDir.toStdString());
        QDir().mkpath(normalizedFallbackDir);
        return normalizedFallbackDir;
    }

    spdlog::warn("PathResolver: {} resolution returned an empty path, using application directory",
                 purpose.toStdString());
    return applicationDirPath_;
}

bool PathResolver::isWritableDirectory(const QString& directoryPath) const
{
    if (directoryPath.isEmpty()) {
        return false;
    }

    QDir directory(directoryPath);
    if (!directory.exists() && !QDir().mkpath(directoryPath)) {
        return false;
    }

    QTemporaryFile probeFile(QDir(directoryPath).filePath(QString::fromLatin1(kProbeTemplate)));
    probeFile.setAutoRemove(true);
    if (!probeFile.open()) {
        return false;
    }

    return true;
}

QString PathResolver::currentApplicationDirPath()
{
    const auto* app = QCoreApplication::instance();
    return app == nullptr ? QString() : QCoreApplication::applicationDirPath();
}

QString PathResolver::currentApplicationName()
{
    const auto* app = QCoreApplication::instance();
    return (app == nullptr || app->applicationName().isEmpty())
               ? QString::fromLatin1(kDefaultApplicationName)
               : app->applicationName();
}

} // namespace infra::platform
