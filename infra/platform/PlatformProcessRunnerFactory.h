/**
 * @file PlatformProcessRunnerFactory.h
 * @brief Creates the default platform-specific elevated process runner.
 */

#pragma once

#include <memory>

#if defined(Q_OS_WIN)
#include "infra/platform/win/Win32ProcessRunner.h"
#endif

namespace infra::platform {

class IPlatformProcessRunner;

#if defined(Q_OS_WIN)
[[nodiscard]] inline std::unique_ptr<IPlatformProcessRunner> createPlatformProcessRunner()
{
    return std::make_unique<Win32ProcessRunner>();
}
#else
[[nodiscard]] inline std::unique_ptr<IPlatformProcessRunner> createPlatformProcessRunner()
{
    return nullptr; // Not supported on this platform
}
#endif

} // namespace infra::platform
