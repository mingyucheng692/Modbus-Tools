/**
 * @file PlatformEncoding.cpp
 * @brief Implements Windows-specific spdlog path encoding.
 */

#include "infra/platform/PlatformEncoding.h"

#include <QDir>

namespace infra::platform {

spdlog::filename_t encodePathForSpdlog(const QString& path)
{
#if defined(SPDLOG_WCHAR_FILENAMES)
    return QDir::toNativeSeparators(path).toStdWString();
#else
    return QDir::toNativeSeparators(path).toStdString();
#endif
}

} // namespace infra::platform
