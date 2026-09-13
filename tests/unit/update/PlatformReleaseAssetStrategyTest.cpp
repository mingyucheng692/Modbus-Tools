/**
 * @file PlatformReleaseAssetStrategyTest.cpp
 * @brief Unit tests for release asset layout guidance and full-package
 *        resolution (T3 data chain: url + published sha256 digest).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>

#include "../../../core/update/PlatformReleaseAssetStrategy.h"

#include <QJsonArray>
#include <QJsonObject>

using core::update::UpdateGuidance;
namespace ra = core::update::release_asset;

namespace {

/// Builds one GitHub release asset object.
QJsonObject makeAsset(const QString& name, const QString& url, const QString& digest = QString())
{
    QJsonObject asset;
    asset.insert(QStringLiteral("name"), name);
    asset.insert(QStringLiteral("browser_download_url"), url);
    if (!digest.isEmpty()) {
        asset.insert(QStringLiteral("digest"), digest);
    }
    return asset;
}

constexpr const char* kLinuxTarGzName = "Modbus-Tools-v1.2.3-linux-x86_64.tar.gz";
constexpr const char* kLinuxTarGzUrl = "https://example.com/Modbus-Tools-v1.2.3-linux-x86_64.tar.gz";

} // namespace

TEST(PlatformReleaseAssetStrategy, GuidanceValuesComeFromLayoutTable)
{
    EXPECT_EQ(ra::layoutForPackage(QStringLiteral("1.2.3"), QStringLiteral("windows-x86_64")).guidance,
              UpdateGuidance::AutomaticInstaller);
    EXPECT_EQ(ra::layoutForPackage(QStringLiteral("1.2.3"), QStringLiteral("linux-x86_64")).guidance,
              UpdateGuidance::TerminalCommand);
    EXPECT_EQ(ra::layoutForPackage(QStringLiteral("1.2.3"), QStringLiteral("macos-arm64")).guidance,
              UpdateGuidance::OpenDownloadPage);
    // Unknown platforms fall back to the least intrusive, historical behavior.
    EXPECT_EQ(ra::layoutForPackage(QStringLiteral("1.2.3"), QStringLiteral("freebsd-x86")).guidance,
              UpdateGuidance::OpenDownloadPage);
}

TEST(PlatformReleaseAssetStrategy, FullPackageDigestIsExtractedToLowercaseHex)
{
    const auto layout = ra::layoutForPackage(QStringLiteral("1.2.3"), QStringLiteral("linux-x86_64"));
    QJsonArray assets;
    assets.append(makeAsset(QLatin1String(kLinuxTarGzName), QLatin1String(kLinuxTarGzUrl),
                            QStringLiteral("sha256:ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789")));

    const auto resolved = ra::resolveFullPackageUrl(assets, layout,
                                                    QStringLiteral("https://example.com/releases/tag/v1.2.3"));
    EXPECT_EQ(resolved.url, QLatin1String(kLinuxTarGzUrl));
    EXPECT_EQ(resolved.sha256,
              QStringLiteral("abcdef0123456789abcdef0123456789abcdef0123456789abcdef0123456789"));
}

TEST(PlatformReleaseAssetStrategy, FullPackageWithoutDigestKeepsUrlAndEmptySha)
{
    const auto layout = ra::layoutForPackage(QStringLiteral("1.2.3"), QStringLiteral("linux-x86_64"));
    QJsonArray assets;
    assets.append(makeAsset(QLatin1String(kLinuxTarGzName), QLatin1String(kLinuxTarGzUrl)));

    const auto resolved = ra::resolveFullPackageUrl(assets, layout,
                                                    QStringLiteral("https://example.com/releases/tag/v1.2.3"));
    EXPECT_EQ(resolved.url, QLatin1String(kLinuxTarGzUrl));
    EXPECT_TRUE(resolved.sha256.isEmpty());
}

TEST(PlatformReleaseAssetStrategy, MalformedDigestYieldsEmptyShaNotGarbage)
{
    const auto layout = ra::layoutForPackage(QStringLiteral("1.2.3"), QStringLiteral("linux-x86_64"));
    QJsonArray assets;
    // Truncated digest must not leak partial data into the command layer.
    // Only the tar.gz is offered, so the resolver matches it (priority order
    // would otherwise hit an earlier-format asset first).
    assets.append(makeAsset(QLatin1String(kLinuxTarGzName), QLatin1String(kLinuxTarGzUrl),
                            QStringLiteral("sha256:abc123")));

    const auto resolved = ra::resolveFullPackageUrl(assets, layout,
                                                    QStringLiteral("https://example.com/releases/tag/v1.2.3"));
    EXPECT_EQ(resolved.url, QLatin1String(kLinuxTarGzUrl));
    EXPECT_TRUE(resolved.sha256.isEmpty());
}

TEST(PlatformReleaseAssetStrategy, FallbackToReleasePageCarriesEmptySha)
{
    const auto layout = ra::layoutForPackage(QStringLiteral("1.2.3"), QStringLiteral("linux-x86_64"));
    QJsonArray assets;
    assets.append(makeAsset(QStringLiteral("Modbus-Tools-v1.2.3-windows-x86_64-Setup.exe"),
                            QStringLiteral("https://example.com/Setup.exe")));

    const QString releaseUrl = QStringLiteral("https://example.com/releases/tag/v1.2.3");
    const auto resolved = ra::resolveFullPackageUrl(assets, layout, releaseUrl);
    EXPECT_EQ(resolved.url, releaseUrl);
    EXPECT_TRUE(resolved.sha256.isEmpty());
}
