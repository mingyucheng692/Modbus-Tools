/**
 * @file PathResolver.cpp
 * @brief Implements writable path resolution for installed and portable deployments.
 */

#include "infra/platform/PathResolver.h"

#include "infra/platform/QtStandardPlatformPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <algorithm>
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

PathResolver::PathResolver()
    : PathResolver(std::make_shared<QtStandardPlatformPaths>(),
                   currentApplicationDirPath(),
                   currentApplicationArguments(),
                   currentApplicationName())
{
}

PathResolver::PathResolver(std::shared_ptr<const IPlatformPaths> platformPaths,
                           QString applicationDirPath,
                           QStringList arguments,
                           QString applicationName)
    : platformPaths_(std::move(platformPaths)),
      applicationDirPath_(normalizedPath(applicationDirPath)),
      applicationName_(applicationName.isEmpty() ? QString::fromLatin1(kDefaultApplicationName)
                                                 : std::move(applicationName))
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

    spdlog::info("PathResolver: portable mode = {}", portableMode_);
}

QString PathResolver::resolveLogDir() const
{
    // Preferred: portable → exe/logs; installed → AppDataLocation/logs.
    // Secondary (portable only): standard log location as fallback before temp.
    // Installed mode never falls back to applicationDirPath_/logs (per
    // AGENTS.md §21.7).
    const QString portableLogDir = QDir(applicationDirPath_).filePath(QString::fromLatin1(kLogsDirectoryName));
    const QString standardLogDir = QDir(platformPaths_->appDataLocation()).filePath(QString::fromLatin1(kLogsDirectoryName));
    const QString preferred = portableMode_ ? portableLogDir : standardLogDir;
    const QString secondary = portableMode_ ? standardLogDir : QString();
    return resolveWritableDir(QStringLiteral("log directory"),
                              preferred,
                              secondary,
                              resolveScopedTempDir() + QStringLiteral("/") + QString::fromLatin1(kLogsDirectoryName));
}

QString PathResolver::resolveConfigDir() const
{
    // Preferred: portable → exe dir; installed → AppConfigLocation.
    // Secondary (portable only): standard config location as fallback before temp.
    // Installed mode never falls back to applicationDirPath_ (per AGENTS.md §21.7:
    // fallback chain is portable → QStandardPaths → TempLocation, never exe-dir
    // without explicit opt-in).
    const QString standardConfigDir = platformPaths_->appConfigLocation();
    const QString preferred = portableMode_ ? applicationDirPath_ : standardConfigDir;
    const QString secondary = portableMode_ ? standardConfigDir : QString();
    return resolveWritableDir(QStringLiteral("config directory"),
                              preferred,
                              secondary,
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

QStringList PathResolver::currentApplicationArguments()
{
    const auto* app = QCoreApplication::instance();
    return app == nullptr ? QStringList() : QCoreApplication::arguments();
}

} // namespace infra::platform
