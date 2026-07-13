/**
 * @file IPlatformUpdateStrategy.h
 * @brief Platform abstraction interface for OTA update operations.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <cstdint>
#include <string>

namespace updater {

/**
 * @brief Platform abstraction for OTA update operations.
 *
 * Each platform (Windows/Linux/macOS) provides its own implementation.
 * All paths and messages use UTF-8 encoded std::string.
 *
 * Platform implementations:
 * - Windows: Win32UpdateStrategy (BCrypt, MoveFileExW, CreateProcessW)
 * - Linux:   LinuxUpdateStrategy (future, OpenSSL/POSIX)
 * - macOS:   MacosUpdateStrategy (future, CommonCrypto/.app bundle)
 */
class IPlatformUpdateStrategy {
public:
    virtual ~IPlatformUpdateStrategy() = default;

    /// Reads entire file content as UTF-8 bytes. Returns empty string on failure.
    virtual std::string readAllBytes(const std::string& path) = 0;

    /// Computes SHA-256 hex digest (lowercase, 64 chars). Returns false on failure.
    virtual bool computeSha256(const std::string& filePath, std::string& sha256) = 0;

    /// Checks if a regular file exists at the given path.
    virtual bool fileExists(const std::string& path) = 0;

    /// Atomically moves/replaces a file. Returns false on failure.
    virtual bool moveFileAtomic(const std::string& source, const std::string& destination) = 0;

    /// Launches the target executable. Returns false on failure.
    virtual bool launchTarget(const std::string& targetPath) = 0;

    /// Waits for the launcher process (by PID) to exit. Returns false on timeout.
    virtual bool waitForLauncherExit(std::uint32_t pid) = 0;

    /// Displays an error message dialog to the user.
    virtual void showError(const std::string& message) = 0;
};

} // namespace updater
