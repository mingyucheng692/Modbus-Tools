/**
 * @file UpdateTask.h
 * @brief Platform-independent update task configuration model.
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
 * @brief Platform-independent update task configuration (parsed from JSON).
 *
 * All paths are UTF-8 encoded. Platform implementations are responsible
 * for converting to native encoding (e.g. UTF-16 on Windows) as needed.
 */
struct UpdateTask {
    int schemaVersion = 0;
    std::uint32_t launcherPid = 0;
    std::string targetExePath;
    std::string newExePath;
    std::string backupExePath;
    std::string expectedVersion;
    std::string expectedSha256;
    bool restartAfterUpdate = true;
};

/**
 * @brief Parses a JSON config string (UTF-8) into UpdateTask.
 * @param json UTF-8 encoded JSON string
 * @param config Output parameter for parsed configuration
 * @return true on success, false on parse error or invalid fields
 */
bool parseTaskConfig(const std::string& json, UpdateTask& config);

} // namespace updater
