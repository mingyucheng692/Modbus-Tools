// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QUrl>
#include <QString>

namespace core::update {

// Role: Pure generator producing Linux terminal command sequences for application updates.
// Thread-safety: Thread-safe (stateless pure functions).
// Ownership: Value semantics; produces QString without persistent resource ownership.

/// Generates the three-segment Linux terminal command sequence that applies a
/// full-package update in place:
///   1) download (cd ~ + curl -LO <url>)
///   2) verify checksum (echo "<sha>  <asset>" | sha256sum -c)
///   3) replace (cd <installDir> + cp backup + tar --strip-components=1)
///
/// Pure function over its inputs; the asset name is derived from the URL via
/// QUrl::fileName() and must therefore point at a file asset (GitHub
/// browser_download_url always does).
///
/// |expectedSha256| empty means the release published no digest: the checksum
/// segment degrades to an explicit WARNING comment line — verification is
/// never silently skipped and there is never a "| sh" style shell pipe.
///
/// Paths match the packaged layout produced by
/// scripts/packaging/deploy-linux.sh: the binary sits at the package root
/// (no bin/ prefix), hence "cp Modbus-Tools Modbus-Tools.old".
[[nodiscard]] QString buildLinuxUpdateCommand(const QUrl& downloadUrl,
                                              const QString& expectedSha256);

} // namespace core::update
