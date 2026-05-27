/**
 * @file PlatformEncoding.cpp
 * @brief Implements macOS-specific spdlog path encoding.
 */

#include "infra/platform/PlatformEncoding.h"

#include <QDir>

namespace infra::platform {

spdlog::filename_t encodePathForSpdlog(const QString& path)
{
    return QDir::toNativeSeparators(path).toStdString();
}

} // namespace infra::platform
