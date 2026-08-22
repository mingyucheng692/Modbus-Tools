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
// (info vs warn) in Logger.cpp. Call sites use the SPDLOG_* macros so that
// SPDLOG_ACTIVE_LEVEL (Debug -> SPDLOG_LEVEL_DEBUG, otherwise
// SPDLOG_LEVEL_INFO, see modbus_tools_apply_logging_policy in the root
// CMakeLists.txt) strips debug-level statements at compile time.
//
// Note on QT_MESSAGELOGCONTEXT: intentionally NOT defined; the Qt message
// handler's file/line context is only reliable for Qt-internal warnings,
// which is why LogBridge does not rely on it.

namespace logging {

[[nodiscard]] bool Init(const QString& logDir, QString* errorMessage = nullptr) noexcept;

void SetLogLevel(spdlog::level::level_enum level) noexcept;

[[nodiscard]] spdlog::level::level_enum GetLogLevel() noexcept;

}
