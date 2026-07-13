/**
 * @file LinuxUpdateStrategy.h
 * @brief Placeholder for Linux OTA update strategy (not yet implemented).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 *
 * TODO: Implement LinuxUpdateStrategy when OTA support is added for Linux.
 * - SHA-256: use OpenSSL or /usr/bin/sha256sum
 * - Atomic file replace: use rename(2) (atomic on same filesystem)
 * - Process launch: use fork/exec or posix_spawn
 * - Path encoding: UTF-8 std::string (native on Linux)
 * - Update target: AppImage or .deb/.rpm package
 */

#pragma once

// #include "../IPlatformUpdateStrategy.h"
//
// namespace updater {
//
// class LinuxUpdateStrategy : public IPlatformUpdateStrategy {
//     // TODO: implement all methods
// };
//
// } // namespace updater
