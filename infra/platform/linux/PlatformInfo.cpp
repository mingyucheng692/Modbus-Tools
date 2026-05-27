/**
 * @file PlatformInfo.cpp
 * @brief Implements Linux-specific platform identification.
 */

#include "infra/platform/PlatformInfo.h"

namespace infra::platform {

QString platformDisplayName()
{
    return QStringLiteral("Linux");
}

} // namespace infra::platform
