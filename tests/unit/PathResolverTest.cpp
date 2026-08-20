/**
 * @file PathResolverTest.cpp
 * @brief Tests the portable-only (exe-directory) path resolution.
 */

#include "infra/platform/PathResolver.h"

#include <QDir>
#include <QTemporaryDir>
#include <gtest/gtest.h>

TEST(PathResolver, ResolvesAllDirsRelativeToExeDirectory)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath(QStringLiteral("app"));
    ASSERT_TRUE(QDir().mkpath(appDir));
    const infra::platform::PathResolver resolver(appDir);

    EXPECT_EQ(QDir::cleanPath(appDir).toStdString(),
              QDir::cleanPath(resolver.applicationDirPath()).toStdString());
    EXPECT_EQ(QDir::cleanPath(appDir).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(appDir).filePath(QStringLiteral("logs"))).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
    EXPECT_EQ(QDir::cleanPath(QDir(appDir).filePath(QStringLiteral("update"))).toStdString(),
              QDir::cleanPath(resolver.resolveUpdateStagingDir()).toStdString());
}

TEST(PathResolver, ResolvesDirsForRootedExeDirectoryWithoutCreatingThem)
{
    // Resolution is pure path concatenation: directories that do not exist
    // yet are still resolved (callers mkpath on demand), and no probing or
    // fallback to user-profile locations ever happens.
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString appDir = QDir(sandbox.path()).filePath(QStringLiteral("not-created"));
    const infra::platform::PathResolver resolver(appDir);

    EXPECT_FALSE(QDir(resolver.resolveLogDir()).exists());
    EXPECT_FALSE(QDir(resolver.resolveUpdateStagingDir()).exists());
    EXPECT_EQ(QDir::cleanPath(QDir(appDir).filePath(QStringLiteral("logs"))).toStdString(),
              QDir::cleanPath(resolver.resolveLogDir()).toStdString());
}

TEST(PathResolver, CleansInjectedApplicationDirPath)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    const QString dirtyPath = sandbox.path() + QStringLiteral("/app/./");
    const infra::platform::PathResolver resolver(dirtyPath);

    EXPECT_EQ(QDir::cleanPath(QDir(sandbox.path()).filePath(QStringLiteral("app"))).toStdString(),
              QDir::cleanPath(resolver.applicationDirPath()).toStdString());
    EXPECT_EQ(QDir::cleanPath(resolver.applicationDirPath()).toStdString(),
              QDir::cleanPath(resolver.resolveConfigDir()).toStdString());
}
