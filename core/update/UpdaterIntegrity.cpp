/**
 * @file UpdaterIntegrity.cpp
 * @brief Implements updater binary integrity verification.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UpdaterIntegrity.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

namespace core::update {

QString computeFileSha256(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return {};
    }
    return QString::fromLatin1(hash.result().toHex());
}

bool verifyUpdaterIntegrity(const QString& updaterPath, const QString& expectedSha256, QString& errorDetail)
{
    if (!QFileInfo::exists(updaterPath)) {
        errorDetail = QStringLiteral("Updater binary not found at %1").arg(updaterPath);
        return false;
    }

    if (expectedSha256.isEmpty()) {
        // No expected hash configured — skip verification (development builds).
        return true;
    }

    const QString actualSha = computeFileSha256(updaterPath);
    if (actualSha.isEmpty()) {
        errorDetail = QStringLiteral("Failed to compute updater SHA256");
        return false;
    }

    if (actualSha.compare(expectedSha256.trimmed(), Qt::CaseInsensitive) != 0) {
        errorDetail = QStringLiteral("Updater integrity check failed. Expected: %1, Actual: %2")
                          .arg(expectedSha256.trimmed(), actualSha);
        return false;
    }

    return true;
}

} // namespace core::update
