/**
 * @file PathResolverTest.cpp
 * @brief Tests writable path resolution for portable deployments.
 */

#include "infra/platform/PathResolver.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <gtest/gtest.h>
#include <utility>

namespace {

infra::platform::PathResolver makeResolver(const QString& appDataDir,
                                           const QString& appConfigDir,
                                           const QString& tempRootDir,
                                           const QString& applicationDir,
                                           QStringList arguments = {})
{
    auto appDataFn = [appDataDir]() { return appDataDir; };
    auto appConfigFn = [appConfigDir]() { return appConfigDir; };
    auto tempFn = [tempRootDir]() { return tempRootDir; };
    return infra::platform::PathResolver(appDataFn, appConfigFn, tempFn,
                                         applicationDir, std::move(arguments),
                                         QStringLiteral("Modbus-Tools-Test"));
}

QString createBlockedPath(const QString& rootPath, const QString& fileName)
{
    const QString blockedPath = QDir(rootPath).filePath(fileName);
    QFile file(blockedPath);
    EXPECT_TRUE(file.open(QIODevice::WriteOnly));
    file.write("blocked");
    file.close();
    return blockedPath;
}

} // namespace

TEST(PathResolver, NonPortableModeUsesStandardInstallDirs)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("app");
    const QString dataDir = QDir(sandbox.path()).filePath("data");
    const QString configDir = QDir(sandbox.path()).filePath("config");
    const QString tempRoot = QDir(sandbox.path()).filePath("temp-root");
    ASSERT_TRUE(QDir().mkpath(appDir));
    ASSERT_TRUE(QDir().mkpath(dataDir));
    ASSERT_TRUE(QDir().mkpath(configDir));
    ASSERT_TRUE(QDir().mkpath(tempRoot));
    const infra::platform::PathResolver resolver =
        makeResolver(dataDir, configDir, tempRoot, appDir);

    EXPECT_FALSE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(configDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(dataDir).filePath("logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, PortableFlagUsesAppDir)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("portable-app");
    ASSERT_TRUE(QDir().mkpath(appDir));
    const QString dataDir = QDir(sandbox.path()).filePath("data");
    const QString configDir = QDir(sandbox.path()).filePath("config");
    const QString tempRoot = QDir(sandbox.path()).filePath("temp-root");
    ASSERT_TRUE(QDir().mkpath(dataDir));
    ASSERT_TRUE(QDir().mkpath(configDir));
    ASSERT_TRUE(QDir().mkpath(tempRoot));
    const infra::platform::PathResolver resolver =
        makeResolver(dataDir, configDir, tempRoot, appDir, {"--portable"});

    EXPECT_TRUE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(appDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(appDir).filePath("logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, PortableMarkerUsesAppDir)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("portable-marker");
    ASSERT_TRUE(QDir().mkpath(appDir));
    const QString dataDir = QDir(sandbox.path()).filePath("data");
    const QString configDir = QDir(sandbox.path()).filePath("config");
    const QString tempRoot = QDir(sandbox.path()).filePath("temp-root");
    ASSERT_TRUE(QDir().mkpath(dataDir));
    ASSERT_TRUE(QDir().mkpath(configDir));
    ASSERT_TRUE(QDir().mkpath(tempRoot));
    QFile markerFile(QDir(appDir).filePath(".portable"));
    ASSERT_TRUE(markerFile.open(QIODevice::WriteOnly));
    markerFile.close();

    const infra::platform::PathResolver resolver =
        makeResolver(dataDir, configDir, tempRoot, appDir);

    EXPECT_TRUE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(appDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
}

TEST(PathResolver, PortableModeFallsBackToTempWhenAppDirIsNotWritable)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString blockedAppPath = createBlockedPath(sandbox.path(), "blocked-app");
    const QString dataDir = QDir(sandbox.path()).filePath("data");
    const QString configDir = QDir(sandbox.path()).filePath("config");
    const QString tempRoot = QDir(sandbox.path()).filePath("temp-root");
    ASSERT_TRUE(QDir().mkpath(dataDir));
    ASSERT_TRUE(QDir().mkpath(configDir));
    ASSERT_TRUE(QDir().mkpath(tempRoot));
    const infra::platform::PathResolver resolver =
        makeResolver(dataDir, configDir, tempRoot, blockedAppPath, {"--portable"});

    const QString expectedTempDir = QDir::cleanPath(
        QDir(tempRoot).filePath("Modbus-Tools-Test"));
    EXPECT_EQ(expectedTempDir.toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(expectedTempDir).filePath("logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, StandardInstallFallsBackToTempWhenStandardDirsAreNotWritable)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("app");
    const QString blockedDataPath = createBlockedPath(sandbox.path(), "blocked-data");
    const QString blockedConfigPath = createBlockedPath(sandbox.path(), "blocked-config");
    const QString tempRoot = QDir(sandbox.path()).filePath("temp-root");
    ASSERT_TRUE(QDir().mkpath(appDir));
    ASSERT_TRUE(QDir().mkpath(tempRoot));
    const infra::platform::PathResolver resolver =
        makeResolver(blockedDataPath, blockedConfigPath, tempRoot, appDir);

    EXPECT_FALSE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(QDir(tempRoot).filePath("Modbus-Tools-Test")).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(QDir(tempRoot).filePath("Modbus-Tools-Test")).filePath("logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, TempDirResolvesToScopedTemp)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("app");
    const QString dataDir = QDir(sandbox.path()).filePath("data");
    const QString configDir = QDir(sandbox.path()).filePath("config");
    const QString tempRoot = QDir(sandbox.path()).filePath("temp-root");
    ASSERT_TRUE(QDir().mkpath(appDir));
    ASSERT_TRUE(QDir().mkpath(dataDir));
    ASSERT_TRUE(QDir().mkpath(configDir));
    ASSERT_TRUE(QDir().mkpath(tempRoot));
    const infra::platform::PathResolver resolver =
        makeResolver(dataDir, configDir, tempRoot, appDir);

    const QString expectedTemp = QDir::cleanPath(
        QDir(tempRoot).filePath("Modbus-Tools-Test"));
    EXPECT_EQ(expectedTemp.toStdString(),
              QDir::cleanPath(resolver.resolveTempDir()).toStdString());
}
