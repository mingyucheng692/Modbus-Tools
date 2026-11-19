// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include <memory>
#include <string_view>
#include <spdlog/spdlog.h>

// Verbose runtime logging is gated by the CMake option
// MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS, which controls spdlog flush level
// (info vs warn) in Logger.cpp. Call sites use the SPDLOG_* macros;
// SPDLOG_ACTIVE_LEVEL is defined as SPDLOG_LEVEL_DEBUG across targets
// (see modbus_tools_apply_logging_policy in the root CMakeLists.txt),
// allowing runtime filtering via SetLogLevel().
//
// Note on QT_MESSAGELOGCONTEXT: intentionally NOT defined; the Qt message
// handler's file/line context is only reliable for Qt-internal warnings.

namespace logging {

[[nodiscard]] bool Init(const QString& logDir, QString* errorMessage = nullptr) noexcept;

/// Uninstalls the Qt message handler, flushes queued log entries, and shuts down spdlog.
void Shutdown() noexcept;

/// Dumps recent in-memory backtrace ring buffer to log sinks (up to 64 records).
void DumpBacktrace() noexcept;

/// Synchronously dumps backtrace, flushes all loggers, terminates spdlog, and aborts.
[[noreturn]] void FatalFlushAndAbort(std::string_view message) noexcept;

/// Returns true if MODBUS_TOOLS_LOG_LEVEL environment variable is set.
[[nodiscard]] bool HasEnvLogLevelOverride() noexcept;

void SetLogLevel(spdlog::level::level_enum level) noexcept;

[[nodiscard]] spdlog::level::level_enum GetLogLevel() noexcept;

}
