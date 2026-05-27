/**
 * @file PlatformEncoding.h
 * @brief Declares the spdlog filename conversion helper.
 */

#pragma once

#include <QString>
#include <spdlog/common.h>

namespace infra::platform {

[[nodiscard]] spdlog::filename_t encodePathForSpdlog(const QString& path);

} // namespace infra::platform
