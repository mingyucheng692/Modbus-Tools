/**
 * @file PlatformInfo.cpp
 * @brief Implements macOS-specific platform identification.
 */

#include "infra/platform/PlatformInfo.h"

namespace infra::platform {

QString platformDisplayName()
{
    return QStringLiteral("macOS");
}

} // namespace infra::platform
