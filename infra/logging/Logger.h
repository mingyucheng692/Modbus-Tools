/**
 * @file Logger.h
 * @brief Header file for Logger.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <memory>
#include <spdlog/spdlog.h>

// Verbose runtime logging is gated by the CMake option
// MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS, which controls spdlog flush level
// (info vs warn) in Logger.cpp. Call sites use spdlog::debug() directly.

namespace logging {

[[nodiscard]] bool Init(const QString& logDir, QString* errorMessage = nullptr) noexcept;

}
