// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "infra/platform/IPlatformProcessRunner.h"

#if defined(Q_OS_WIN)
#include "infra/platform/win/Win32ProcessRunner.h"
#endif

namespace infra::platform {

std::unique_ptr<IPlatformProcessRunner> createDefaultPlatformProcessRunner()
{
#if defined(Q_OS_WIN)
    return std::make_unique<Win32ProcessRunner>();
#else
    return nullptr;
#endif
}

} // namespace infra::platform
