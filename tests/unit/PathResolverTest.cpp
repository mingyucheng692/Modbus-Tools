/**
 * @file PathResolverTest.cpp
 * @brief Tests writable path resolution for portable deployments.
 */

#include "infra/platform/PathResolver.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <gtest/gtest.h>

namespace {

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

TEST(PathResolver, NonPortableModeUsesAppDir)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("app");
    ASSERT_TRUE(QDir().mkpath(appDir));
    const infra::platform::PathResolver resolver(appDir, {}, "Modbus-Tools-Test");

    EXPECT_FALSE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(appDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(appDir).filePath("logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, PortableFlagUsesAppDir)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("portable-app");
    ASSERT_TRUE(QDir().mkpath(appDir));
    const infra::platform::PathResolver resolver(appDir, {"--portable"}, "Modbus-Tools-Test");

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
    QFile markerFile(QDir(appDir).filePath(".portable"));
    ASSERT_TRUE(markerFile.open(QIODevice::WriteOnly));
    markerFile.close();

    const infra::platform::PathResolver resolver(appDir, {}, "Modbus-Tools-Test");

    EXPECT_TRUE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(appDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
}

TEST(PathResolver, UnwritableAppDirFallsBackToTemp)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString blockedAppPath = createBlockedPath(sandbox.path(), "blocked-app");
    const infra::platform::PathResolver resolver(blockedAppPath, {"--portable"}, "Modbus-Tools-Test");

    const QString expectedTempDir = QDir::cleanPath(
        QDir(QDir::tempPath()).filePath("Modbus-Tools-Test"));
    EXPECT_EQ(expectedTempDir.toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir(expectedTempDir).filePath("logs").toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, MacAppBundleUsesAppDir)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("Modbus-Tools.app/Contents/MacOS");
    ASSERT_TRUE(QDir().mkpath(appDir));
    const infra::platform::PathResolver resolver(appDir, {}, "Modbus-Tools-Test");

    EXPECT_FALSE(resolver.isPortableMode());
    // App bundle dir is writable, so it should be used directly.
    EXPECT_EQ(QDir::cleanPath(appDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(appDir).filePath("logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, TempDirResolvesToScopedTemp)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("app");
    ASSERT_TRUE(QDir().mkpath(appDir));
    const infra::platform::PathResolver resolver(appDir, {}, "Modbus-Tools-Test");

    const QString expectedTemp = QDir::cleanPath(
        QDir(QDir::tempPath()).filePath("Modbus-Tools-Test"));
    EXPECT_EQ(expectedTemp.toStdString(),
              QDir::cleanPath(resolver.resolveTempDir()).toStdString());
}