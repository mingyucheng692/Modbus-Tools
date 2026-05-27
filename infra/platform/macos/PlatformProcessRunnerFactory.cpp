/**
 * @file PlatformProcessRunnerFactory.cpp
 * @brief Creates the macOS fallback process runner.
 */

#include "infra/platform/PlatformProcessRunnerFactory.h"

#include "infra/platform/UnsupportedProcessRunner.h"

#include <memory>

namespace infra::platform {

std::unique_ptr<IPlatformProcessRunner> createPlatformProcessRunner()
{
    return std::make_unique<UnsupportedProcessRunner>();
}

} // namespace infra::platform
