/**
 * @file UpdaterIntegrityTest.cpp
 * @brief Unit tests for updater binary integrity verification.
 *
 * Covers the five verifyUpdaterIntegrity() branches: missing file, empty
 * expected hash (skip), unreadable file, hash mismatch, hash match. Runs on
 * every platform — the implementation is pure QtCore.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gtest/gtest.h>

#include "../../../core/update/UpdaterIntegrity.h"

#include <QCryptographicHash>
#include <QFile>
#include <QTemporaryDir>

using core::update::computeFileSha256;
using core::update::verifyUpdaterIntegrity;

namespace {

/// Reference SHA256 of arbitrary content, computed independently of the
/// function under test via QCryptographicHash directly.
QString sha256Of(const QByteArray& content)
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(content);
    return QString::fromLatin1(hash.result().toHex());
}

/// Writes content to a new file inside the temporary directory and returns
/// its full path; returns an empty string on write failure.
QString writeTempFile(const QTemporaryDir& dir, const QString& name, const QByteArray& content)
{
    const QString path = dir.filePath(name);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return {};
    }
    if (file.write(content) != content.size()) {
        return {};
    }
    return path;
}

constexpr const char* kUpdaterContent = "MZ fake updater binary payload";

} // namespace

TEST(UpdaterIntegrity, MissingFileFails)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString missingPath = dir.filePath(QStringLiteral("no-such-updater.exe"));

    QString detail;
    EXPECT_FALSE(verifyUpdaterIntegrity(missingPath, sha256Of(kUpdaterContent), detail));
    EXPECT_TRUE(detail.contains(QStringLiteral("not found")));
}

TEST(UpdaterIntegrity, EmptyExpectedHashSkipsCheck)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = writeTempFile(dir, QStringLiteral("updater.exe"), kUpdaterContent);
    ASSERT_FALSE(path.isEmpty());

    QString detail;
    EXPECT_TRUE(verifyUpdaterIntegrity(path, QString(), detail));
    EXPECT_TRUE(detail.isEmpty());
}

TEST(UpdaterIntegrity, UnreadableFileFails)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    // A directory path exists but cannot be opened for reading as a file on
    // any platform, forcing computeFileSha256() to fail.
    QString detail;
    EXPECT_FALSE(verifyUpdaterIntegrity(dir.path(), sha256Of(kUpdaterContent), detail));
    EXPECT_TRUE(detail.contains(QStringLiteral("Failed to compute")));
}

TEST(UpdaterIntegrity, HashMismatchFails)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = writeTempFile(dir, QStringLiteral("updater.exe"), kUpdaterContent);
    ASSERT_FALSE(path.isEmpty());

    const QByteArray tampered = "MZ tampered updater binary payload";
    QString detail;
    EXPECT_FALSE(verifyUpdaterIntegrity(path, sha256Of(tampered), detail));
    EXPECT_TRUE(detail.contains(QStringLiteral("integrity check failed")));
}

TEST(UpdaterIntegrity, HashMatchPassesCaseInsensitively)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = writeTempFile(dir, QStringLiteral("updater.exe"), kUpdaterContent);
    ASSERT_FALSE(path.isEmpty());

    const QString expectedHash = sha256Of(kUpdaterContent);
    QString detail;
    EXPECT_TRUE(verifyUpdaterIntegrity(path, expectedHash, detail));
    EXPECT_TRUE(verifyUpdaterIntegrity(path, expectedHash.toUpper(), detail));
    // Tolerate surrounding whitespace, mirroring CI-produced hash files.
    EXPECT_TRUE(verifyUpdaterIntegrity(path, "  " + expectedHash + " ", detail));
}

TEST(UpdaterIntegrity, ComputeFileSha256MatchesReference)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = writeTempFile(dir, QStringLiteral("sample.bin"), kUpdaterContent);
    ASSERT_FALSE(path.isEmpty());

    EXPECT_EQ(computeFileSha256(path), sha256Of(kUpdaterContent));
}

TEST(UpdaterIntegrity, ComputeFileSha256MissingFileReturnsEmpty)
{
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    EXPECT_TRUE(computeFileSha256(dir.filePath(QStringLiteral("missing.bin"))).isEmpty());
}
