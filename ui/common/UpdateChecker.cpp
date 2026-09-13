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
#include "../../core/update/UpdateManager.h"

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
#include <QtGlobal>
#include <algorithm>
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

}

namespace ui::common {

UpdateChecker::UpdateChecker(QObject* parent)
    : QObject(parent),
      networkManager_(new QNetworkAccessManager(this)) {
}

void UpdateChecker::checkForUpdates() {
    SPDLOG_INFO("UpdateChecker: Checking for updates at {}", MODBUS_TOOLS_RELEASES_API_URL);
    QNetworkRequest request(QUrl(QStringLiteral(MODBUS_TOOLS_RELEASES_API_URL)));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Modbus-Tools/%1").arg(currentVersion()));
    request.setRawHeader("Accept", "application/vnd.github+json");

    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            const QString error = reply->errorString();
            SPDLOG_ERROR("UpdateChecker: Network error checking for updates: {}", error.toStdString());
            emit checkFailed(error);
            return;
        }

        const QByteArray rawData = reply->readAll();
        const std::string jsonStr(rawData.constData(), rawData.size());

        auto releases = core::update::release_parser::parseReleases(jsonStr,
                                                                    includePrereleaseOptIn());
        if (releases.empty()) {
            emit noUpdateAvailable(currentVersion());
            return;
        }

        // The "/releases" list endpoint orders by creation date, which is not
        // guaranteed to match version order (e.g. a backport of an older
        // branch). Pick the highest version among the filtered releases so the
        // list endpoint preserves the former "/releases/latest" semantics.
        const auto latestRelease = std::max_element(
            releases.cbegin(), releases.cend(),
            [](const core::update::ReleaseInfo& lhs, const core::update::ReleaseInfo& rhs) {
                return core::update::release_parser::compareVersions(lhs.tagName, rhs.tagName) < 0;
            });

        const QString latestVersion = QString::fromStdString(
            core::update::release_parser::normalizeVersion(latestRelease->tagName));
        if (latestVersion.isEmpty()) {
            emit checkFailed(tr("Release tag is missing"));
            return;
        }

        QString releaseUrl = QString::fromStdString(latestRelease->htmlUrl);
        if (releaseUrl.isEmpty()) {
            releaseUrl = releasePageUrl();
        }

        // Parse assets using Qt's JSON (more reliable for asset parsing)
        const QJsonDocument document = QJsonDocument::fromJson(
            QByteArray::fromStdString(latestRelease->jsonBody));
        const QJsonObject root = document.object();
        const QJsonArray assets = root.value("assets").toArray();

        QString updateOnlyUrl;
        QString updateOnlySha256;
        QString checksumsUrl;
        QString fullPackageUrl;
        const auto artifactLayout = core::update::release_asset::layoutForPackage(latestVersion, packagePlatform());
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

        fullPackageUrl = core::update::release_asset::resolveFullPackageUrl(assets, artifactLayout, releaseUrl);

        const std::string currentVer = currentVersion().toStdString();
        const int compareResult = core::update::release_parser::compareVersions(
            latestRelease->tagName, currentVer);
        if (compareResult > 0) {
            SPDLOG_INFO("UpdateChecker: New version available: v{} (Current: v{})",
                         latestVersion.toStdString(), currentVer);
            emit updateAvailable(core::update::UpdateInfo{
                currentVersion(),
                latestVersion,
                updateOnlyUrl,
                updateOnlySha256,
                checksumsUrl,
                fullPackageUrl,
                releaseUrl
            });
            return;
        }

        SPDLOG_INFO("UpdateChecker: No new version available (Current: v{})", currentVer);
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

bool UpdateChecker::includePrereleaseOptIn() {
    if (!qEnvironmentVariableIsSet("MODBUS_TOOLS_PRERELEASE")) {
        return false;
    }
    const QString value = qEnvironmentVariable("MODBUS_TOOLS_PRERELEASE").trimmed();
    return value.compare(QLatin1String("1"), Qt::CaseInsensitive) == 0 ||
           value.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
}

} // namespace ui::common
