/**
 * @file PlatformReleaseAssetStrategy.cpp
 * @brief Implements platform-specific OTA release asset layout helpers.
 */

#include "PlatformReleaseAssetStrategy.h"

#include <QJsonObject>
#include <QRegularExpression>

#ifndef MODBUS_TOOLS_PLATFORM
#define MODBUS_TOOLS_PLATFORM "windows-x86_64"
#endif

namespace core::update::release_asset {

namespace {

QString normalizedPackagePlatform(const QString& packagePlatform)
{
    return packagePlatform.trimmed().toLower();
}

QString buildPlatformAssetName(const QString& version,
                               const QString& packagePlatform,
                               const QString& suffix)
{
    return QStringLiteral("Modbus-Tools-v%1-%2%3").arg(version, packagePlatform, suffix);
}

UpdatePlatformFamily familyFromPackagePlatform(const QString& packagePlatform)
{
    const QString normalized = normalizedPackagePlatform(packagePlatform);
    if (normalized.startsWith(QStringLiteral("windows-"))) {
        return UpdatePlatformFamily::Windows;
    }
    if (normalized.startsWith(QStringLiteral("macos-"))) {
        return UpdatePlatformFamily::MacOs;
    }
    if (normalized.startsWith(QStringLiteral("linux-"))) {
        return UpdatePlatformFamily::Linux;
    }
    return UpdatePlatformFamily::Unknown;
}

} // namespace

QString bundledUpdaterBinaryName(const QString& packagePlatform)
{
    // Same family dispatch as the release asset names — keep both in sync.
    switch (familyFromPackagePlatform(packagePlatform)) {
    case UpdatePlatformFamily::Windows:
        return QStringLiteral("updater.exe");
    case UpdatePlatformFamily::MacOs:
    case UpdatePlatformFamily::Linux:
    case UpdatePlatformFamily::Unknown:
    default:
        return QStringLiteral("updater");
    }
}

PlatformUpdateArtifactLayout layoutForPackage(const QString& version,
                                              const QString& packagePlatform)
{
    PlatformUpdateArtifactLayout layout;
    layout.packagePlatform = packagePlatform;
    layout.family = familyFromPackagePlatform(packagePlatform);

    switch (layout.family) {
    case UpdatePlatformFamily::Windows:
        layout.updateOnlyAssetName = buildPlatformAssetName(version, packagePlatform,
                                                            QStringLiteral("-UpdateOnly.exe"));
        layout.fullPackageAssetNames = {
            buildPlatformAssetName(version, packagePlatform, QStringLiteral("-Setup.exe"))
        };
        layout.guidance = UpdateGuidance::AutomaticInstaller;
        break;
    case UpdatePlatformFamily::MacOs:
        layout.fullPackageAssetNames = {
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".dmg")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".pkg")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".zip"))
        };
        layout.guidance = UpdateGuidance::OpenDownloadPage;
        break;
    case UpdatePlatformFamily::Linux:
        layout.fullPackageAssetNames = {
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".AppImage")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".deb")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".rpm")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".tar.gz"))
        };
        layout.guidance = UpdateGuidance::TerminalCommand;
        break;
    case UpdatePlatformFamily::Unknown:
    default:
        break;
    }

    return layout;
}

QString digestSha256(const QJsonObject& asset)
{
    static const QRegularExpression digestPattern(QStringLiteral("^sha256:([a-fA-F0-9]{64})$"));
    const QString digestRaw = asset.value(QStringLiteral("digest")).toString().trimmed();
    const QRegularExpressionMatch match = digestPattern.match(digestRaw);
    return match.hasMatch() ? match.captured(1).toLower() : QString();
}

ResolvedFullPackage resolveFullPackageUrl(const QJsonArray& assets,
                                          const PlatformUpdateArtifactLayout& layout,
                                          const QString& releaseUrl)
{
    for (const QString& expectedAssetName : layout.fullPackageAssetNames) {
        for (const QJsonValue& assetValue : assets) {
            if (!assetValue.isObject()) {
                continue;
            }

            const QJsonObject asset = assetValue.toObject();
            const QString assetName = asset.value(QStringLiteral("name")).toString();
            if (assetName.compare(expectedAssetName, Qt::CaseInsensitive) != 0) {
                continue;
            }

            const QString assetUrl = asset.value(QStringLiteral("browser_download_url")).toString();
            if (!assetUrl.isEmpty()) {
                return ResolvedFullPackage{assetUrl, digestSha256(asset)};
            }
        }
    }

    // No package asset matched — fall back to the release page. The empty
    // sha256 is surfaced downstream as an explicit checksum-unavailable
    // warning, never as a silently skipped verification.
    return ResolvedFullPackage{releaseUrl, QString()};
}

} // namespace core::update::release_asset
