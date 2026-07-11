/**
 * @file PlatformEncoding.h
 * @brief Declares the spdlog filename conversion helper.
 */

#pragma once

#include <QString>
#include <QDir>
#include <spdlog/common.h>

namespace infra::platform {

[[nodiscard]] inline spdlog::filename_t encodePathForSpdlog(const QString& path)
{
#if defined(SPDLOG_WCHAR_FILENAMES)
    return QDir::toNativeSeparators(path).toStdWString();
#else
    return QDir::toNativeSeparators(path).toStdString();
#endif
}

} // namespace infra::platform
