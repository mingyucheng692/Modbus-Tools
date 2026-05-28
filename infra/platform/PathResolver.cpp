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

constexpr auto kPortableArg = "--portable";
constexpr auto kPortableMarkerFileName = ".portable";
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
                   currentArguments(),
                   currentApplicationName())
{
}

PathResolver::PathResolver(std::shared_ptr<const IPlatformPaths> platformPaths,
                           QString applicationDirPath,
                           QStringList arguments,
                           QString applicationName)
    : platformPaths_(std::move(platformPaths)),
      applicationDirPath_(normalizedPath(applicationDirPath)),
      arguments_(std::move(arguments)),
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
    const QString standardLogDir = QDir(platformPaths_->appDataLocation()).filePath(QString::fromLatin1(kLogsDirectoryName));
    const QString portableLogDir = QDir(applicationDirPath_).filePath(QString::fromLatin1(kLogsDirectoryName));
    return resolveWritableDir(QStringLiteral("log directory"),
                              isPortableMode() ? portableLogDir : standardLogDir,
                              standardLogDir,
                              resolveScopedTempDir() + QStringLiteral("/") + QString::fromLatin1(kLogsDirectoryName));
}

QString PathResolver::resolveConfigDir() const
{
    const QString standardConfigDir = platformPaths_->appConfigLocation();
    return resolveWritableDir(QStringLiteral("config directory"),
                              isPortableMode() ? applicationDirPath_ : standardConfigDir,
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

bool PathResolver::isPortableMode() const
{
    return arguments_.contains(QString::fromLatin1(kPortableArg)) || hasPortableMarker();
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

bool PathResolver::hasPortableMarker() const
{
    return QFile::exists(portableMarkerPath());
}

QString PathResolver::portableMarkerPath() const
{
    return QDir(applicationDirPath_).filePath(QString::fromLatin1(kPortableMarkerFileName));
}

QString PathResolver::currentApplicationDirPath()
{
    const auto* app = QCoreApplication::instance();
    return app == nullptr ? QString() : QCoreApplication::applicationDirPath();
}

QStringList PathResolver::currentArguments()
{
    const auto* app = QCoreApplication::instance();
    return app == nullptr ? QStringList() : QCoreApplication::arguments();
}

QString PathResolver::currentApplicationName()
{
    const auto* app = QCoreApplication::instance();
    return (app == nullptr || app->applicationName().isEmpty())
               ? QString::fromLatin1(kDefaultApplicationName)
               : app->applicationName();
}

} // namespace infra::platform
