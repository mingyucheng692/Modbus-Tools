/**
 * @file PathResolver.cpp
 * @brief Implements writable path resolution for portable deployments.
 */

#include "infra/platform/PathResolver.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <algorithm>
#include <utility>
#include <spdlog/spdlog.h>

namespace {

constexpr auto kLogsDirectoryName = "logs";
constexpr auto kPortableMarkerFileName = ".portable";
constexpr auto kPortableCliArgument = "--portable";
constexpr auto kProbeTemplate = ".path-resolver-write-test-XXXXXX";
constexpr auto kDefaultApplicationName = "Modbus-Tools";

QString normalizedPath(const QString& path)
{
    return QDir::cleanPath(path);
}

} // namespace

namespace infra::platform {

// ---------------------------------------------------------------------------
// PathResolver
// ---------------------------------------------------------------------------

PathResolver::PathResolver()
    : PathResolver(currentAppDataDirPath,
                   currentAppConfigDirPath,
                   currentTempRootDirPath,
                   currentApplicationDirPath(),
                   currentApplicationArguments(),
                   currentApplicationName())
{
}

PathResolver::PathResolver(QString applicationDirPath,
                           QStringList arguments,
                           QString applicationName)
    : PathResolver(currentAppDataDirPath,
                   currentAppConfigDirPath,
                   currentTempRootDirPath,
                   std::move(applicationDirPath),
                   std::move(arguments),
                   std::move(applicationName))
{
}

PathResolver::PathResolver(StandardPathProvider appDataDirProvider,
                           StandardPathProvider appConfigDirProvider,
                           StandardPathProvider tempDirProvider,
                           QString applicationDirPath,
                           QStringList arguments,
                           QString applicationName)
    : applicationDirPath_(normalizedPath(applicationDirPath)),
      applicationName_(applicationName.isEmpty() ? QString::fromLatin1(kDefaultApplicationName)
                                                 : std::move(applicationName)),
      appDataDirProvider_(std::move(appDataDirProvider)),
      appConfigDirProvider_(std::move(appConfigDirProvider)),
      tempDirProvider_(std::move(tempDirProvider))
{
    detectPortableMode(arguments);
}

void PathResolver::detectPortableMode(const QStringList& arguments)
{
    // Per AGENTS.md §21.7: portable mode MUST be explicit opt-in.
    // Never infer from "exe dir happens to be writable".
    const QString markerPath = QDir(applicationDirPath_).filePath(QString::fromLatin1(kPortableMarkerFileName));
    if (QFileInfo::exists(markerPath)) {
        portableMode_ = true;
    }

    if (!portableMode_) {
        const auto end = arguments.end();
        const auto it = std::find(arguments.begin(), end, QString::fromLatin1(kPortableCliArgument));
        if (it != end) {
            portableMode_ = true;
        }
    }

    SPDLOG_INFO("PathResolver: portable mode = {}", portableMode_);
}

QString PathResolver::resolveLogDir() const
{
    const QString preferredLogDir = portableMode_
        ? QDir(applicationDirPath_).filePath(QString::fromLatin1(kLogsDirectoryName))
        : QDir(appDataDirProvider_ ? appDataDirProvider_() : QString())
              .filePath(QString::fromLatin1(kLogsDirectoryName));
    const QString fallbackLogDir = QDir(resolveScopedTempDir()).filePath(QString::fromLatin1(kLogsDirectoryName));
    return resolveWritableDir(QStringLiteral("log directory"),
                              preferredLogDir,
                              fallbackLogDir);
}

QString PathResolver::resolveConfigDir() const
{
    const QString preferredConfigDir = portableMode_
        ? applicationDirPath_
        : (appConfigDirProvider_ ? appConfigDirProvider_() : QString());
    return resolveWritableDir(QStringLiteral("config directory"),
                              preferredConfigDir,
                              resolveScopedTempDir());
}

QString PathResolver::resolveTempDir() const
{
    return resolveWritableDir(QStringLiteral("temporary directory"),
                              resolveScopedTempDir(),
                              applicationDirPath_);
}

QString PathResolver::resolveScopedTempDir() const
{
    const QString tempRoot = tempDirProvider_ ? tempDirProvider_() : QString();
    return QDir(tempRoot).filePath(applicationName_);
}

QString PathResolver::resolveWritableDir(const QString& purpose,
                                         const QString& preferredDir,
                                         const QString& fallbackDir) const
{
    const QString normalizedPreferredDir = normalizedPath(preferredDir);
    if (!normalizedPreferredDir.isEmpty() && isWritableDirectory(normalizedPreferredDir)) {
        return normalizedPreferredDir;
    }

    if (!normalizedPreferredDir.isEmpty()) {
        SPDLOG_WARN("PathResolver: {} is not writable, falling back from {}",
                     purpose.toStdString(),
                     normalizedPreferredDir.toStdString());
    }

    const QString normalizedFallbackDir = normalizedPath(fallbackDir);
    if (!normalizedFallbackDir.isEmpty() && isWritableDirectory(normalizedFallbackDir)) {
        return normalizedFallbackDir;
    }

    if (!normalizedFallbackDir.isEmpty()) {
        SPDLOG_WARN("PathResolver: {} fallback is not writable, using best-effort {}",
                     purpose.toStdString(),
                     normalizedFallbackDir.toStdString());
        QDir().mkpath(normalizedFallbackDir);
        return normalizedFallbackDir;
    }

    SPDLOG_WARN("PathResolver: {} resolution returned an empty path, using application directory",
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

QString PathResolver::currentAppDataDirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}

QString PathResolver::currentAppConfigDirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
}

QString PathResolver::currentApplicationName()
{
    const auto* app = QCoreApplication::instance();
    return (app == nullptr || app->applicationName().isEmpty())
               ? QString::fromLatin1(kDefaultApplicationName)
               : app->applicationName();
}

QStringList PathResolver::currentApplicationArguments()
{
    const auto* app = QCoreApplication::instance();
    return app == nullptr ? QStringList() : QCoreApplication::arguments();
}

QString PathResolver::currentTempRootDirPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
}

} // namespace infra::platform
