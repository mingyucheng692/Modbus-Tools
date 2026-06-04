/**
 * @file PlatformReleaseAssetStrategy.cpp
 * @brief Implements platform-specific OTA release asset layout helpers.
 */

#include "PlatformReleaseAssetStrategy.h"

#include <QJsonObject>

#ifndef MODBUS_TOOLS_PLATFORM
#define MODBUS_TOOLS_PLATFORM "windows-x86_64"
#endif

namespace core::update {

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

} // namespace

PlatformUpdateArtifactLayout PlatformReleaseAssetStrategy::currentLayout(const QString& version)
{
    return layoutForPackage(version, QStringLiteral(MODBUS_TOOLS_PLATFORM));
}

PlatformUpdateArtifactLayout PlatformReleaseAssetStrategy::layoutForPackage(const QString& version,
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
        break;
    case UpdatePlatformFamily::MacOs:
        layout.fullPackageAssetNames = {
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".dmg")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".pkg")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".zip"))
        };
        break;
    case UpdatePlatformFamily::Linux:
        layout.fullPackageAssetNames = {
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".AppImage")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".deb")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".rpm")),
            buildPlatformAssetName(version, packagePlatform, QStringLiteral(".tar.gz"))
        };
        break;
    case UpdatePlatformFamily::Unknown:
    default:
        break;
    }

    return layout;
}

QString PlatformReleaseAssetStrategy::resolveFullPackageUrl(const QJsonArray& assets,
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
                return assetUrl;
            }
        }
    }

    return releaseUrl;
}

UpdatePlatformFamily PlatformReleaseAssetStrategy::familyFromPackagePlatform(const QString& packagePlatform)
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

} // namespace core::update
