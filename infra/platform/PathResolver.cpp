/**
 * @file PathResolver.cpp
 * @brief Implements writable path resolution for the portable-only deployment.
 */

#include "infra/platform/PathResolver.h"

#include <QCoreApplication>
#include <QDir>
#include <utility>

namespace {

constexpr auto kLogsDirectoryName = "logs";
constexpr auto kUpdateStagingDirectoryName = "update";

} // namespace

namespace infra::platform {

PathResolver::PathResolver()
    : PathResolver([]() -> QString {
          const auto* app = QCoreApplication::instance();
          return app == nullptr ? QString() : QCoreApplication::applicationDirPath();
      }())
{
}

PathResolver::PathResolver(QString applicationDirPath)
    : applicationDirPath_(QDir::cleanPath(std::move(applicationDirPath)))
{
}

QString PathResolver::resolveLogDir() const
{
    return QDir(applicationDirPath_).filePath(QString::fromLatin1(kLogsDirectoryName));
}

QString PathResolver::resolveConfigDir() const
{
    return applicationDirPath_;
}

QString PathResolver::resolveUpdateStagingDir() const
{
    return QDir(applicationDirPath_).filePath(QString::fromLatin1(kUpdateStagingDirectoryName));
}

} // namespace infra::platform
