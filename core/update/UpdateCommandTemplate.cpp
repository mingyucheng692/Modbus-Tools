/**
 * @file UpdateCommandTemplate.cpp
 * @brief Implements the Linux terminal update command generator (方案 C).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UpdateCommandTemplate.h"

#include <QCoreApplication>

namespace core::update {

namespace {

// Portable install root agreed for the 方案 C flow (plan §2). The generated
// comment tells users to adjust it when they installed elsewhere.
const QString kDefaultInstallDir = QStringLiteral("~/.local/opt/modbus-tools");

/// The asset the URL points at; drives both the checksum line and the tar
/// input path. Derived from the URL so callers cannot pass a mismatched name.
QString assetNameFromUrl(const QUrl& downloadUrl)
{
    return downloadUrl.fileName();
}

} // namespace

QString buildLinuxUpdateCommand(const QUrl& downloadUrl, const QString& expectedSha256)
{
    const QString url = downloadUrl.toString(QUrl::FullyEncoded);
    const QString assetName = assetNameFromUrl(downloadUrl);
    const QString trimmedSha = expectedSha256.trimmed();

    // Segment 2: verification. An absent checksum degrades to an explicit
    // WARNING comment line — deliberately visible, never silently skipped.
    const QString checksumSegment = trimmedSha.isEmpty()
        ? QCoreApplication::translate("core::update::UpdateCommandTemplate",
              "# WARNING: no checksum was published for this release; "
              "verify the download source manually before continuing.")
        : QStringLiteral("echo \"%1  %2\" | sha256sum -c")
              .arg(trimmedSha, assetName);

    QString command;
    command += QCoreApplication::translate("core::update::UpdateCommandTemplate",
                                           "# 1) Download\n");
    // Start from a known directory: curl -LO writes the asset here and the
    // later segments reference it via "~/".
    command += QStringLiteral("cd ~\n");
    command += QStringLiteral("curl -LO %1\n\n").arg(url);

    command += QCoreApplication::translate("core::update::UpdateCommandTemplate",
                                           "# 2) Verify checksum (do not continue on failure)\n");
    command += checksumSegment;
    command += QLatin1Char('\n');

    command += QCoreApplication::translate("core::update::UpdateCommandTemplate",
              "# 3) Replace (quit Modbus-Tools first; adjust the path below if you "
              "installed elsewhere)\n");
    command += QStringLiteral("cd %1\n").arg(kDefaultInstallDir);
    command += QStringLiteral("cp Modbus-Tools Modbus-Tools.old\n");
    command += QStringLiteral("tar -xzf ~/%1 --strip-components=1\n").arg(assetName);

    return command;
}

} // namespace core::update
