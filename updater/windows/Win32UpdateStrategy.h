// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <cstdint>
#include <string>

namespace updater {

/// Windows implementation using Win32 API (BCrypt, MoveFileExW, CreateProcessW).
class Win32UpdateStrategy {
public:
    bool computeSha256(const std::string& filePath, std::string& sha256);
    bool fileExists(const std::string& path);
    bool moveFileAtomic(const std::string& source, const std::string& destination);
    bool launchTarget(const std::string& targetPath);
    bool waitForLauncherExit(std::uint32_t pid);
    void showError(const std::string& message);
};

} // namespace updater
