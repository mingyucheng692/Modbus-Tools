/**
 * @file PlatformInfo.cpp
 * @brief Implements Windows-specific platform identification.
 */

#include "infra/platform/PlatformInfo.h"

namespace infra::platform {

QString platformDisplayName()
{
    return QStringLiteral("Windows");
}

} // namespace infra::platform
