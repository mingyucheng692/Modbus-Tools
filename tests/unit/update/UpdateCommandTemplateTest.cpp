/**
 * @file UpdateCommandTemplateTest.cpp
 * @brief Unit tests for the Linux terminal update command generator (T4).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>

#include "../../../core/update/UpdateCommandTemplate.h"

using core::update::buildLinuxUpdateCommand;

namespace {

constexpr const char* kAssetUrl =
    "https://github.com/mingyucheng692/Modbus-Tools/releases/download/v1.2.3/"
    "Modbus-Tools-v1.2.3-linux-x86_64.tar.gz";
constexpr const char* kAssetName = "Modbus-Tools-v1.2.3-linux-x86_64.tar.gz";
constexpr const char* kSha256 =
    "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

/// The anti-pattern guard is "no pipe into a shell", i.e. no command line
/// ENDS with a shell pipe. A naive substring check for "| sh" would false-
/// trip on the legitimate "| sha256sum -c" segment.
[[nodiscard]] bool noShellPipeAtLineEnd(const QString& command)
{
    const QStringList lines = command.split(QLatin1Char('\n'));
    for (const QString& rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.endsWith(QStringLiteral("| sh")) || line.endsWith(QStringLiteral("| bash"))) {
            return false;
        }
    }
    return true;
}

} // namespace

TEST(UpdateCommandTemplate, EmitsThreeSegmentsWithAllElements)
{
    const QString command = buildLinuxUpdateCommand(QUrl(QLatin1String(kAssetUrl)),
                                                    QLatin1String(kSha256));

    // Segment 1: known CWD + download.
    EXPECT_TRUE(command.contains(QStringLiteral("cd ~")));
    EXPECT_TRUE(command.contains(QStringLiteral("curl -LO ") + QLatin1String(kAssetUrl)));
    // Segment 2: checksum verification with the published digest.
    EXPECT_TRUE(command.contains(QLatin1String(kSha256)));
    EXPECT_TRUE(command.contains(QStringLiteral(" | sha256sum -c")));
    // Segment 3: in-place replace aligned with the package-root layout.
    EXPECT_TRUE(command.contains(QStringLiteral("cp Modbus-Tools Modbus-Tools.old")));
    EXPECT_TRUE(command.contains(QStringLiteral("tar -xzf ~/") + QLatin1String(kAssetName)));
    EXPECT_TRUE(command.contains(QStringLiteral("--strip-components=1")));
    EXPECT_TRUE(command.contains(QStringLiteral("~/.local/opt/modbus-tools")));
}

TEST(UpdateCommandTemplate, AssetNameIsDerivedFromUrl)
{
    // Two-param signature: the generator owns the asset name (QUrl::fileName()).
    const QString command = buildLinuxUpdateCommand(QUrl(QLatin1String(kAssetUrl)),
                                                    QLatin1String(kSha256));
    EXPECT_TRUE(command.contains(QLatin1String(kAssetName)));
}

TEST(UpdateCommandTemplate, PackageRootLayoutHasNoBinPrefix)
{
    const QString command = buildLinuxUpdateCommand(QUrl(QLatin1String(kAssetUrl)),
                                                    QLatin1String(kSha256));
    EXPECT_FALSE(command.contains(QStringLiteral("bin/Modbus-Tools")));
}

TEST(UpdateCommandTemplate, NoShellPipeAnywhere)
{
    const QString withSha = buildLinuxUpdateCommand(QUrl(QLatin1String(kAssetUrl)),
                                                    QLatin1String(kSha256));
    EXPECT_TRUE(noShellPipeAtLineEnd(withSha));

    const QString withoutSha = buildLinuxUpdateCommand(QUrl(QLatin1String(kAssetUrl)), QString());
    EXPECT_TRUE(noShellPipeAtLineEnd(withoutSha));
}

TEST(UpdateCommandTemplate, EmptyShaDegradesToExplicitWarningNotSilentSkip)
{
    const QString command = buildLinuxUpdateCommand(QUrl(QLatin1String(kAssetUrl)), QString());

    EXPECT_TRUE(command.contains(QStringLiteral("WARNING")));
    EXPECT_FALSE(command.contains(QStringLiteral("sha256sum")));
    // Download and replace segments remain fully usable.
    EXPECT_TRUE(command.contains(QStringLiteral("curl -LO ") + QLatin1String(kAssetUrl)));
    EXPECT_TRUE(command.contains(QStringLiteral("cp Modbus-Tools Modbus-Tools.old")));
    EXPECT_TRUE(command.contains(QStringLiteral("--strip-components=1")));
}

TEST(UpdateCommandTemplate, BlankShaIsTreatedAsEmpty)
{
    const QString command = buildLinuxUpdateCommand(QUrl(QLatin1String(kAssetUrl)),
                                                    QStringLiteral("   "));
    EXPECT_TRUE(command.contains(QStringLiteral("WARNING")));
    EXPECT_FALSE(command.contains(QStringLiteral("sha256sum")));
}
