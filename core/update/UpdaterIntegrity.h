/**
 * @file UpdaterIntegrity.h
 * @brief SHA256 integrity verification helpers for the bundled updater binary.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>

namespace core::update {

/// Computes SHA256 of a file. Returns empty string on failure.
/// Pure QtCore implementation (QFile + QCryptographicHash), available on all
/// platforms and unit-testable without a Windows environment.
[[nodiscard]] QString computeFileSha256(const QString& filePath);

/// Verifies the updater binary integrity before launch.
/// Returns true if the check passes or is skipped (no expected hash configured).
/// On failure returns false and fills @p errorDetail with a diagnostic string.
/// The expected hash is injected at build time via MODBUS_TOOLS_UPDATER_SHA256;
/// an empty value skips verification (development / non-Windows builds without
/// an updater binary).
[[nodiscard]] bool verifyUpdaterIntegrity(const QString& updaterPath,
                                          const QString& expectedSha256,
                                          QString& errorDetail);

} // namespace core::update
