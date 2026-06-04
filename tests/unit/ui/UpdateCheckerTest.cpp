#include <gtest/gtest.h>

#include "../../../core/update/PlatformReleaseAssetStrategy.h"
#include "../../../ui/common/UpdateChecker.h"

#include <QJsonArray>
#include <QJsonObject>

TEST(UpdateCheckerAssetSelection, WindowsLayoutKeepsDedicatedUpdateOnlyAndSetupContracts)
{
    const auto layout = core::update::PlatformReleaseAssetStrategy::layoutForPackage(
        QStringLiteral("1.2.3"), QStringLiteral("windows-x86_64"));

    EXPECT_EQ(layout.updateOnlyAssetName,
              QStringLiteral("Modbus-Tools-v1.2.3-windows-x86_64-UpdateOnly.exe"));
    EXPECT_EQ(layout.fullPackageAssetNames, QStringList({
                  QStringLiteral("Modbus-Tools-v1.2.3-windows-x86_64-Setup.exe")
              }));
}

TEST(UpdateCheckerAssetSelection, MacOsAssetsPreferDiskImageBeforeArchiveFallback)
{
    const auto layout = core::update::PlatformReleaseAssetStrategy::layoutForPackage(
        QStringLiteral("1.2.3"), QStringLiteral("macos-arm64"));

    EXPECT_TRUE(layout.updateOnlyAssetName.isEmpty());
    EXPECT_EQ(layout.fullPackageAssetNames,
              QStringList({
                  QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64.dmg"),
                  QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64.pkg"),
                  QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64.zip")
              }));

    QJsonArray assets;
    QJsonObject zipAsset;
    zipAsset.insert(QStringLiteral("name"), QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64.zip"));
    zipAsset.insert(QStringLiteral("browser_download_url"), QStringLiteral("https://example.com/macos-arm64.zip"));
    assets.append(zipAsset);

    QJsonObject dmgAsset;
    dmgAsset.insert(QStringLiteral("name"), QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64.dmg"));
    dmgAsset.insert(QStringLiteral("browser_download_url"), QStringLiteral("https://example.com/macos-arm64.dmg"));
    assets.append(dmgAsset);

    const QString resolvedUrl = ui::common::UpdateChecker::resolveFullPackageUrl(
        assets,
        layout,
        QStringLiteral("https://example.com/releases/tag/v1.2.3"));

    EXPECT_EQ(resolvedUrl, QStringLiteral("https://example.com/macos-arm64.dmg"));
}

TEST(UpdateCheckerAssetSelection, LinuxAssetsAcceptPlatformPackageMatrix)
{
    const auto layout = core::update::PlatformReleaseAssetStrategy::layoutForPackage(
        QStringLiteral("1.2.3"), QStringLiteral("linux-x86_64"));

    EXPECT_TRUE(layout.updateOnlyAssetName.isEmpty());
    EXPECT_EQ(layout.fullPackageAssetNames,
              QStringList({
                  QStringLiteral("Modbus-Tools-v1.2.3-linux-x86_64.AppImage"),
                  QStringLiteral("Modbus-Tools-v1.2.3-linux-x86_64.deb"),
                  QStringLiteral("Modbus-Tools-v1.2.3-linux-x86_64.rpm"),
                  QStringLiteral("Modbus-Tools-v1.2.3-linux-x86_64.tar.gz")
              }));

    QJsonArray assets;
    QJsonObject rpmAsset;
    rpmAsset.insert(QStringLiteral("name"), QStringLiteral("Modbus-Tools-v1.2.3-linux-x86_64.rpm"));
    rpmAsset.insert(QStringLiteral("browser_download_url"), QStringLiteral("https://example.com/linux-x86_64.rpm"));
    assets.append(rpmAsset);

    const QString releaseUrl = QStringLiteral("https://example.com/releases/tag/v1.2.3");
    const QString resolvedUrl = ui::common::UpdateChecker::resolveFullPackageUrl(
        assets,
        layout,
        releaseUrl);

    EXPECT_EQ(resolvedUrl, QStringLiteral("https://example.com/linux-x86_64.rpm"));
}

TEST(UpdateCheckerAssetSelection, ForeignPlatformAssetsFallBackToReleasePage)
{
    const auto layout = core::update::PlatformReleaseAssetStrategy::layoutForPackage(
        QStringLiteral("1.2.3"), QStringLiteral("macos-arm64"));

    QJsonArray assets;
    QJsonObject foreignAsset;
    foreignAsset.insert(QStringLiteral("name"), QStringLiteral("Modbus-Tools-v1.2.3-windows-x86_64-Setup.exe"));
    foreignAsset.insert(QStringLiteral("browser_download_url"), QStringLiteral("https://example.com/windows-x86_64-setup.exe"));
    assets.append(foreignAsset);

    const QString releaseUrl = QStringLiteral("https://example.com/releases/tag/v1.2.3");
    const QString resolvedUrl = ui::common::UpdateChecker::resolveFullPackageUrl(
        assets,
        layout,
        releaseUrl);

    EXPECT_EQ(resolvedUrl, releaseUrl);
}
