/**
 * @file UpdateChecker.cpp
 * @brief Implementation of UpdateChecker.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UpdateChecker.h"

#include "../../core/update/PlatformReleaseAssetStrategy.h"
#include "../../core/update/ReleaseParser.h"

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

        const QByteArray rawData = reply->readAll();
        const std::string jsonStr(rawData.constData(), rawData.size());
        constexpr bool includePrerelease = MODBUS_TOOLS_INCLUDE_PRERELEASE != 0;

        auto releases = core::update::ReleaseParser::parseReleases(jsonStr, includePrerelease);
        if (releases.empty()) {
            emit noUpdateAvailable(currentVersion());
            return;
        }

        const auto& firstRelease = releases.front();
        const QString latestVersion = QString::fromStdString(
            core::update::ReleaseParser::normalizeVersion(firstRelease.tagName));
        if (latestVersion.isEmpty()) {
            emit checkFailed(tr("Release tag is missing"));
            return;
        }

        QString releaseUrl = QString::fromStdString(firstRelease.htmlUrl);
        if (releaseUrl.isEmpty()) {
            releaseUrl = releasePageUrl();
        }

        // Parse assets using Qt's JSON (more reliable for asset parsing)
        const QJsonDocument document = QJsonDocument::fromJson(
            QByteArray::fromStdString(firstRelease.jsonBody));
        const QJsonObject root = document.object();
        const QJsonArray assets = root.value("assets").toArray();

        QString updateOnlyUrl;
        QString updateOnlySha256;
        QString checksumsUrl;
        QString fullPackageUrl;
        const auto artifactLayout = core::update::PlatformReleaseAssetStrategy::layoutForPackage(latestVersion, packagePlatform());
        const QRegularExpression digestPattern(QStringLiteral("^sha256:([a-fA-F0-9]{64})$"));
        for (const QJsonValue& assetValue : assets) {
            if (!assetValue.isObject()) {
                continue;
            }
            const QJsonObject asset = assetValue.toObject();
            const QString assetName = asset.value("name").toString();
            const QString assetUrl = asset.value("browser_download_url").toString();
            if (!artifactLayout.updateOnlyAssetName.isEmpty() &&
                assetName.compare(artifactLayout.updateOnlyAssetName, Qt::CaseInsensitive) == 0) {
                updateOnlyUrl = assetUrl;
                const QString digestRaw = asset.value("digest").toString().trimmed();
                const QRegularExpressionMatch digestMatch = digestPattern.match(digestRaw);
                if (digestMatch.hasMatch()) {
                    updateOnlySha256 = digestMatch.captured(1).toLower();
                }
            } else if (assetName.compare(QStringLiteral("sha256sums.txt"), Qt::CaseInsensitive) == 0) {
                checksumsUrl = assetUrl;
            }
        }

        fullPackageUrl = core::update::PlatformReleaseAssetStrategy::resolveFullPackageUrl(assets, artifactLayout, releaseUrl);

        const std::string currentVer = currentVersion().toStdString();
        const int compareResult = core::update::ReleaseParser::compareVersions(
            firstRelease.tagName, currentVer);
        if (compareResult > 0) {
            spdlog::info("UpdateChecker: New version available: v{} (Current: v{})",
                         latestVersion.toStdString(), currentVer);
            emit updateAvailable(currentVersion(),
                                 latestVersion,
                                 updateOnlyUrl,
                                 updateOnlySha256,
                                 checksumsUrl,
                                 fullPackageUrl,
                                 releaseUrl);
            return;
        }

        spdlog::info("UpdateChecker: No new version available (Current: v{})", currentVer);
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

} // namespace ui::common
