/**
 * @file PlatformProcessRunnerFactory.h
 * @brief Creates the default platform-specific elevated process runner.
 */

#pragma once

#include <memory>

namespace infra::platform {

class IPlatformProcessRunner;

[[nodiscard]] std::unique_ptr<IPlatformProcessRunner> createPlatformProcessRunner();

} // namespace infra::platform
