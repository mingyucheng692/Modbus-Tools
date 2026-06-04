/**
 * @file PlatformReleaseAssetStrategy.h
 * @brief Declares platform-specific OTA release asset layout helpers.
 */

#pragma once

#include <QJsonArray>
#include <QString>
#include <QStringList>

namespace core::update {

enum class UpdatePlatformFamily {
    Unknown = 0,
    Windows,
    MacOs,
    Linux
};

struct PlatformUpdateArtifactLayout {
    UpdatePlatformFamily family = UpdatePlatformFamily::Unknown;
    QString packagePlatform;
    QString updateOnlyAssetName;
    QStringList fullPackageAssetNames;

    [[nodiscard]] bool supportsInAppUpdate() const noexcept
    {
        return !updateOnlyAssetName.isEmpty();
    }
};

class PlatformReleaseAssetStrategy {
public:
    [[nodiscard]] static PlatformUpdateArtifactLayout currentLayout(const QString& version);
    [[nodiscard]] static PlatformUpdateArtifactLayout layoutForPackage(const QString& version,
                                                                      const QString& packagePlatform);
    [[nodiscard]] static QString resolveFullPackageUrl(const QJsonArray& assets,
                                                       const PlatformUpdateArtifactLayout& layout,
                                                       const QString& releaseUrl);

private:
    [[nodiscard]] static UpdatePlatformFamily familyFromPackagePlatform(const QString& packagePlatform);
};

} // namespace core::update
