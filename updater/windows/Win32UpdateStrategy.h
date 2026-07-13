/**
 * @file Win32UpdateStrategy.h
 * @brief Windows implementation of IPlatformUpdateStrategy.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../IPlatformUpdateStrategy.h"

namespace updater {

/// Windows implementation using Win32 API (BCrypt, MoveFileExW, CreateProcessW).
class Win32UpdateStrategy : public IPlatformUpdateStrategy {
public:
    std::string readAllBytes(const std::string& path) override;
    bool computeSha256(const std::string& filePath, std::string& sha256) override;
    bool fileExists(const std::string& path) override;
    bool moveFileAtomic(const std::string& source, const std::string& destination) override;
    bool launchTarget(const std::string& targetPath) override;
    bool waitForLauncherExit(std::uint32_t pid) override;
    void showError(const std::string& message) override;
};

} // namespace updater
