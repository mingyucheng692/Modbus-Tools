/**
 * @file PathResolverTest.cpp
 * @brief Tests writable path resolution for installed and portable deployments.
 */

#include "infra/platform/IPlatformPaths.h"
#include "infra/platform/PathResolver.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <gtest/gtest.h>
#include <memory>

namespace {

class FakePlatformPaths final : public infra::platform::IPlatformPaths {
public:
    FakePlatformPaths(QString appDataLocation, QString appConfigLocation, QString tempLocation)
        : appDataLocation_(std::move(appDataLocation)),
          appConfigLocation_(std::move(appConfigLocation)),
          tempLocation_(std::move(tempLocation))
    {
    }

    [[nodiscard]] QString appDataLocation() const override
    {
        return appDataLocation_;
    }

    [[nodiscard]] QString appConfigLocation() const override
    {
        return appConfigLocation_;
    }

    [[nodiscard]] QString tempLocation() const override
    {
        return tempLocation_;
    }

private:
    QString appDataLocation_;
    QString appConfigLocation_;
    QString tempLocation_;
};

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

TEST(PathResolver, InstalledModePrefersStandardLocations)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("app");
    const QString dataDir = QDir(sandbox.path()).filePath("data");
    const QString configDir = QDir(sandbox.path()).filePath("config");
    const QString tempDir = QDir(sandbox.path()).filePath("temp");

    auto platformPaths = std::make_shared<FakePlatformPaths>(dataDir, configDir, tempDir);
    const infra::platform::PathResolver resolver(platformPaths, appDir, {}, "Modbus-Tools-Test");

    EXPECT_FALSE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(QDir(dataDir).filePath("logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(configDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(tempDir).filePath("Modbus-Tools-Test")).toStdString(),
              QDir::cleanPath(resolver.resolveTempDir()).toStdString());
}

TEST(PathResolver, PortableFlagUsesApplicationLocalPaths)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("portable-app");
    const QString dataDir = QDir(sandbox.path()).filePath("data");
    const QString configDir = QDir(sandbox.path()).filePath("config");
    const QString tempDir = QDir(sandbox.path()).filePath("temp");

    auto platformPaths = std::make_shared<FakePlatformPaths>(dataDir, configDir, tempDir);
    const infra::platform::PathResolver resolver(platformPaths, appDir, {"--portable"}, "Modbus-Tools-Test");

    EXPECT_TRUE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(appDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(appDir).filePath("logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, PortableMarkerUsesApplicationLocalPaths)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("portable-marker");
    ASSERT_TRUE(QDir().mkpath(appDir));
    QFile markerFile(QDir(appDir).filePath(".portable"));
    ASSERT_TRUE(markerFile.open(QIODevice::WriteOnly));
    markerFile.close();

    auto platformPaths = std::make_shared<FakePlatformPaths>(
        QDir(sandbox.path()).filePath("data"),
        QDir(sandbox.path()).filePath("config"),
        QDir(sandbox.path()).filePath("temp"));
    const infra::platform::PathResolver resolver(platformPaths, appDir, {}, "Modbus-Tools-Test");

    EXPECT_TRUE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(appDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
}

TEST(PathResolver, UnwritablePortablePathsFallBackToStandardLocations)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString blockedAppPath = createBlockedPath(sandbox.path(), "portable-blocked");
    const QString dataDir = QDir(sandbox.path()).filePath("data");
    const QString configDir = QDir(sandbox.path()).filePath("config");
    const QString tempDir = QDir(sandbox.path()).filePath("temp");

    auto platformPaths = std::make_shared<FakePlatformPaths>(dataDir, configDir, tempDir);
    const infra::platform::PathResolver resolver(platformPaths, blockedAppPath, {"--portable"}, "Modbus-Tools-Test");

    EXPECT_TRUE(resolver.isPortableMode());
    EXPECT_EQ(QDir::cleanPath(configDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(dataDir).filePath("logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, UnwritableStandardPathsFallBackToTempLocation)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath("app");
    const QString blockedDataPath = createBlockedPath(sandbox.path(), "data-blocked");
    const QString blockedConfigPath = createBlockedPath(sandbox.path(), "config-blocked");
    const QString tempDir = QDir(sandbox.path()).filePath("temp");

    auto platformPaths = std::make_shared<FakePlatformPaths>(blockedDataPath, blockedConfigPath, tempDir);
    const infra::platform::PathResolver resolver(platformPaths, appDir, {}, "Modbus-Tools-Test");

    EXPECT_EQ(QDir::cleanPath(QDir(tempDir).filePath("Modbus-Tools-Test")).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(tempDir).filePath("Modbus-Tools-Test/logs")).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}
