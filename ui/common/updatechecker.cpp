/**
 * @file updatechecker.cpp
 * @brief Implementation of updatechecker.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UpdateChecker.h"

#include <QJsonArray>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStringList>
#include <QUrl>
#include <spdlog/spdlog.h>

namespace {

#ifndef MODBUS_TOOLS_APP_VERSION
#error "MODBUS_TOOLS_APP_VERSION must be defined by CMake"
#endif

#ifndef MODBUS_TOOLS_RELEASES_API_URL
#define MODBUS_TOOLS_RELEASES_API_URL "https://api.github.com/repos/mingyucheng692/Modbus-Tools/releases"
#endif

#ifndef MODBUS_TOOLS_RELEASES_PAGE_URL
#define MODBUS_TOOLS_RELEASES_PAGE_URL "https://github.com/mingyucheng692/Modbus-Tools/releases"
#endif

#ifndef MODBUS_TOOLS_PLATFORM
#define MODBUS_TOOLS_PLATFORM "windows-x86_64"
#endif

#ifndef MODBUS_TOOLS_INCLUDE_PRERELEASE
#define MODBUS_TOOLS_INCLUDE_PRERELEASE 0
#endif

}

namespace ui::common {

UpdateChecker::UpdateChecker(QObject* parent)
    : QObject(parent),
      networkManager_(new QNetworkAccessManager(this)) {
}

void UpdateChecker::checkForUpdates() {
    spdlog::info("UpdateChecker: Checking for updates at {}", MODBUS_TOOLS_RELEASES_API_URL);
    QNetworkRequest request(QUrl(QStringLiteral(MODBUS_TOOLS_RELEASES_API_URL)));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Modbus-Tools/%1").arg(currentVersion()));
    request.setRawHeader("Accept", "application/vnd.github+json");

    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            const QString error = reply->errorString();
            spdlog::error("UpdateChecker: Network error checking for updates: {}", error.toStdString());
            emit checkFailed(error);
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        constexpr bool includePrerelease = MODBUS_TOOLS_INCLUDE_PRERELEASE != 0;
        QJsonObject root;
        if (document.isArray()) {
            const QJsonArray releases = document.array();
            for (const QJsonValue& releaseValue : releases) {
                if (!releaseValue.isObject()) {
                    continue;
                }
                const QJsonObject candidate = releaseValue.toObject();
                if (candidate.value("draft").toBool()) {
                    continue;
                }
                if (!includePrerelease && candidate.value("prerelease").toBool()) {
                    continue;
                }
                root = candidate;
                break;
            }
        } else if (document.isObject()) {
            root = document.object();
            if (root.value("draft").toBool()) {
                root = QJsonObject();
            } else if (root.value("prerelease").toBool() && !includePrerelease) {
                root = QJsonObject();
            }
        }

        if (root.isEmpty()) {
            emit noUpdateAvailable(currentVersion());
            return;
        }

        const QString latestVersion = normalizeVersion(root.value("tag_name").toString());
        if (latestVersion.isEmpty()) {
            emit checkFailed(tr("Release tag is missing"));
            return;
        }

        QString releaseUrl = root.value("html_url").toString();
        if (releaseUrl.isEmpty()) {
            releaseUrl = releasePageUrl();
        }

        QString updateOnlyUrl;
        QString updateOnlySha256;
        QString checksumsUrl;
        QString fullPackageUrl;
        const QString expectedUpdateOnlyAsset = expectedUpdateOnlyName(latestVersion);
        const QString expectedSetupAsset = expectedSetupName(latestVersion);
        const QString expectedZipAsset = expectedZipName(latestVersion);
        const QJsonArray assets = root.value("assets").toArray();
        const QRegularExpression digestPattern(QStringLiteral("^sha256:([a-fA-F0-9]{64})$"));
        for (const QJsonValue& assetValue : assets) {
            if (!assetValue.isObject()) {
                continue;
            }
            const QJsonObject asset = assetValue.toObject();
            const QString assetName = asset.value("name").toString();
            const QString assetUrl = asset.value("browser_download_url").toString();
            if (assetName.compare(expectedUpdateOnlyAsset, Qt::CaseInsensitive) == 0) {
                updateOnlyUrl = assetUrl;
                const QString digestRaw = asset.value("digest").toString().trimmed();
                const QRegularExpressionMatch digestMatch = digestPattern.match(digestRaw);
                if (digestMatch.hasMatch()) {
                    updateOnlySha256 = digestMatch.captured(1).toLower();
                }
            } else if (assetName.compare(QStringLiteral("sha256sums.txt"), Qt::CaseInsensitive) == 0) {
                checksumsUrl = assetUrl;
            } else if (assetName.compare(expectedSetupAsset, Qt::CaseInsensitive) == 0 ||
                       assetName.compare(expectedZipAsset, Qt::CaseInsensitive) == 0) {
                if (fullPackageUrl.isEmpty()) {
                    fullPackageUrl = assetUrl;
                }
            }
        }

        if (fullPackageUrl.isEmpty() && !assets.isEmpty() && assets.first().isObject()) {
            fullPackageUrl = assets.first().toObject().value("browser_download_url").toString();
        }
        if (fullPackageUrl.isEmpty()) {
            fullPackageUrl = releaseUrl;
        }

        const int compareResult = compareVersions(latestVersion, currentVersion());
        if (compareResult > 0) {
            spdlog::info("UpdateChecker: New version available: v{} (Current: v{})", latestVersion.toStdString(), currentVersion().toStdString());
            emit updateAvailable(currentVersion(),
                                 latestVersion,
                                 updateOnlyUrl,
                                 updateOnlySha256,
                                 checksumsUrl,
                                 fullPackageUrl,
                                 releaseUrl);
            return;
        }

        spdlog::info("UpdateChecker: No new version available (Current: v{})", currentVersion().toStdString());
        emit noUpdateAvailable(currentVersion());
    });
}

QString UpdateChecker::currentVersion() {
    const QString appVersion = QCoreApplication::applicationVersion().trimmed();
    if (!appVersion.isEmpty()) {
        return appVersion;
    }
    return QStringLiteral(MODBUS_TOOLS_APP_VERSION);
}

QString UpdateChecker::packagePlatform() {
    return QStringLiteral(MODBUS_TOOLS_PLATFORM);
}

QString UpdateChecker::releasePageUrl() {
    return QStringLiteral(MODBUS_TOOLS_RELEASES_PAGE_URL);
}

QString UpdateChecker::expectedUpdateOnlyName(const QString& version) {
    return QStringLiteral("Modbus-Tools-v%1-%2-UpdateOnly.exe").arg(version, packagePlatform());
}

QString UpdateChecker::expectedSetupName(const QString& version) {
    return QStringLiteral("Modbus-Tools-v%1-%2-Setup.exe").arg(version, packagePlatform());
}

QString UpdateChecker::expectedZipName(const QString& version) {
    return QStringLiteral("Modbus-Tools-v%1-%2.zip").arg(version, packagePlatform());
}

QString UpdateChecker::normalizeVersion(const QString& rawVersion) {
    QString cleaned = rawVersion.trimmed();
    if (cleaned.startsWith('v') || cleaned.startsWith('V')) {
        cleaned = cleaned.mid(1);
    }
    return cleaned;
}

int UpdateChecker::compareVersions(const QString& left, const QString& right) {
    const QStringList leftParts = normalizeVersion(left).split('.');
    const QStringList rightParts = normalizeVersion(right).split('.');
    const int maxParts = qMax(leftParts.size(), rightParts.size());
    for (int i = 0; i < maxParts; ++i) {
        const int leftValue = i < leftParts.size() ? leftParts.at(i).toInt() : 0;
        const int rightValue = i < rightParts.size() ? rightParts.at(i).toInt() : 0;
        if (leftValue > rightValue) {
            return 1;
        }
        if (leftValue < rightValue) {
            return -1;
        }
    }
    return 0;
}

} // namespace ui::common
