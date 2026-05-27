/**
 * @file PlatformProcessRunnerFactory.cpp
 * @brief Creates the Windows process runner.
 */

#include "infra/platform/PlatformProcessRunnerFactory.h"

#include "infra/platform/win/Win32ProcessRunner.h"

#include <memory>

namespace infra::platform {

std::unique_ptr<IPlatformProcessRunner> createPlatformProcessRunner()
{
    return std::make_unique<Win32ProcessRunner>();
}

} // namespace infra::platform
