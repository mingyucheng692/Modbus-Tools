/**
 * @file PlatformInfo.h
 * @brief Declares lightweight platform identification helpers.
 */

#pragma once

#include <QString>

namespace infra::platform {

[[nodiscard]] inline QString platformDisplayName()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("Windows");
#elif defined(Q_OS_MACOS)
    return QStringLiteral("macOS");
#elif defined(Q_OS_LINUX)
    return QStringLiteral("Linux");
#else
    return QStringLiteral("Unknown");
#endif
}

} // namespace infra::platform
