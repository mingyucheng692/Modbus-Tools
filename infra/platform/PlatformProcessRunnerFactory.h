/**
 * @file PlatformProcessRunnerFactory.h
 * @brief Creates the default platform-specific elevated process runner.
 */

#pragma once

#include <memory>

#if defined(Q_OS_WIN)
#include "infra/platform/win/Win32ProcessRunner.h"
#else
#include "infra/platform/UnsupportedProcessRunner.h"
#endif

namespace infra::platform {

class IPlatformProcessRunner;

[[nodiscard]] inline std::unique_ptr<IPlatformProcessRunner> createPlatformProcessRunner()
{
#if defined(Q_OS_WIN)
    return std::make_unique<Win32ProcessRunner>();
#else
    return std::make_unique<UnsupportedProcessRunner>();
#endif
}

} // namespace infra::platform
