// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>

namespace infra::platform {

class CrashHandler final {
public:
    /// Pre-allocates static path buffers and installs platform-specific crash filters:
    /// Windows: SEH UnhandledExceptionFilter + minidump via DbgHelp + text crash log + CRT handlers
    /// POSIX: sigaction (SIGSEGV/SIGABRT/SIGFPE/SIGILL/SIGBUS) + async-signal-safe text crash log
    static void install(const QString& logDir) noexcept;

    CrashHandler() = delete;
};

} // namespace infra::platform
