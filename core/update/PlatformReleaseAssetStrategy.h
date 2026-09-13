/**
 * @file PlatformReleaseAssetStrategy.h
 * @brief Declares platform-specific OTA release asset layout helpers.
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace core::update {

enum class UpdatePlatformFamily {
    Unknown = 0,
    Windows,
    MacOs,
    Linux
};

/// How the client guides the user through applying a full-package update.
/// Part of the layout table (single source of truth for platform deltas):
/// adding a distribution format that can self-update means adding a table
/// entry, never a UI-level OS branch.
enum class UpdateGuidance {
    /// Full-package installer runs itself (Windows Setup.exe / UpdateOnly).
    AutomaticInstaller,
    /// User applies the update via a generated terminal command sequence
    /// (Linux tar.gz on a terminal-shaped distribution).
    TerminalCommand,
    /// No in-app apply path; send the user to the release download page
    /// (macOS GUI-shaped artifacts).
    OpenDownloadPage
};

struct PlatformUpdateArtifactLayout {
    UpdatePlatformFamily family = UpdatePlatformFamily::Unknown;
    QString packagePlatform;
    QString updateOnlyAssetName;
    QStringList fullPackageAssetNames;
    UpdateGuidance guidance = UpdateGuidance::OpenDownloadPage;

    [[nodiscard]] bool supportsInAppUpdate() const noexcept
    {
        return !updateOnlyAssetName.isEmpty();
    }
};

namespace release_asset {

/// Full-package resolution result: the download URL (falling back to the
/// release page when no package asset matched) plus the asset's published
/// SHA256 digest. An empty |sha256| is a deliberate "checksum unavailable"
/// signal — the command layer must surface it explicitly (never silently
/// skip verification).
struct ResolvedFullPackage {
    QString url;
    QString sha256;
};

/// Extracts the lowercase SHA256 hex digest from a GitHub asset's
/// "digest" field ("sha256:<64 hex>"); returns an empty string when the
/// digest is absent or malformed.
[[nodiscard]] QString digestSha256(const QJsonObject& asset);

[[nodiscard]] PlatformUpdateArtifactLayout layoutForPackage(const QString& version,
                                                            const QString& packagePlatform);
[[nodiscard]] ResolvedFullPackage resolveFullPackageUrl(const QJsonArray& assets,
                                                        const PlatformUpdateArtifactLayout& layout,
                                                        const QString& releaseUrl);
/// Bundled updater binary name for the given package platform. Shares the
/// same family dispatch as the release asset names: Windows appends ".exe",
/// other families return the plain "updater" name.
[[nodiscard]] QString bundledUpdaterBinaryName(const QString& packagePlatform);

} // namespace release_asset

} // namespace core::update
