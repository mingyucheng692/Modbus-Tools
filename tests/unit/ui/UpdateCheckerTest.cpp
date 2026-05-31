#include <gtest/gtest.h>

#include "../../../ui/common/UpdateChecker.h"

#include <QJsonArray>
#include <QJsonObject>

class UpdateCheckerAssetSelectionTest : public ::testing::Test {
};

TEST_F(UpdateCheckerAssetSelectionTest, MatchingSetupAssetUsesDirectDownloadUrl)
{
    QJsonArray assets;
    QJsonObject asset;
    asset.insert(QStringLiteral("name"), QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64.zip"));
    asset.insert(QStringLiteral("browser_download_url"), QStringLiteral("https://example.com/macos-arm64.zip"));
    assets.append(asset);

    const QString resolvedUrl = ui::common::UpdateChecker::resolveFullPackageUrl(
        assets,
        QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64-Setup.exe"),
        QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64.zip"),
        QStringLiteral("https://example.com/releases/tag/v1.2.3"));

    EXPECT_EQ(resolvedUrl, QStringLiteral("https://example.com/macos-arm64.zip"));
}

TEST_F(UpdateCheckerAssetSelectionTest, ForeignPlatformAssetsFallBackToReleasePage)
{
    QJsonArray assets;
    QJsonObject foreignAsset;
    foreignAsset.insert(QStringLiteral("name"), QStringLiteral("Modbus-Tools-v1.2.3-windows-x86_64.zip"));
    foreignAsset.insert(QStringLiteral("browser_download_url"), QStringLiteral("https://example.com/windows-x86_64.zip"));
    assets.append(foreignAsset);

    const QString releaseUrl = QStringLiteral("https://example.com/releases/tag/v1.2.3");
    const QString resolvedUrl = ui::common::UpdateChecker::resolveFullPackageUrl(
        assets,
        QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64-Setup.exe"),
        QStringLiteral("Modbus-Tools-v1.2.3-macos-arm64.zip"),
        releaseUrl);

    EXPECT_EQ(resolvedUrl, releaseUrl);
}
