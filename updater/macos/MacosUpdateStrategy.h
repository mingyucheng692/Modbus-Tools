/**
 * @file MacosUpdateStrategy.h
 * @brief Placeholder for macOS OTA update strategy (not yet implemented).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 *
 * TODO: Implement MacosUpdateStrategy when OTA support is added for macOS.
 * - SHA-256: use CommonCrypto (CC_SHA256)
 * - Atomic file replace: use rename(2) or NSFileManager
 * - Process launch: use NSTask or posix_spawn
 * - Path encoding: UTF-8 std::string (native on macOS)
 * - Update target: replace entire .app bundle directory, not single file
 */

#pragma once

// #include "../IPlatformUpdateStrategy.h"
//
// namespace updater {
//
// class MacosUpdateStrategy : public IPlatformUpdateStrategy {
//     // TODO: implement all methods
// };
//
// } // namespace updater
