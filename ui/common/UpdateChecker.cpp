// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "UpdateChecker.h"

#include "../../core/Config.h"
#include "../../core/update/PlatformReleaseAssetStrategy.h"
#include "../../core/update/ReleaseParser.h"
#include "../../core/update/UpdateManager.h"

#include <QJsonArray>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
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

core::update::UpdateInfo buildUpdateInfo(const core::update::ReleaseInfo& release,
                                         const QString& currentVer,
                                         const QString& latestVersion,
                                         const QString& platform,
                                         const QString& fallbackPageUrl) {
    QString releaseUrl = QString::fromStdString(release.htmlUrl);
    if (releaseUrl.isEmpty()) {
        releaseUrl = fallbackPageUrl;
    }

    const QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromStdString(release.jsonBody));
    const QJsonArray assets = document.object().value("assets").toArray();
    const auto artifactLayout = core::update::release_asset::layoutForPackage(latestVersion, platform);

    QString updateOnlyUrl;
    QString updateOnlySha256;
    QString checksumsUrl;
    for (const QJsonValue& assetValue : assets) {
        if (!assetValue.isObject()) continue;
        const QJsonObject asset = assetValue.toObject();
        const QString assetName = asset.value("name").toString();
        const QString assetUrl = asset.value("browser_download_url").toString();
        if (!artifactLayout.updateOnlyAssetName.isEmpty() &&
            assetName.compare(artifactLayout.updateOnlyAssetName, Qt::CaseInsensitive) == 0) {
            updateOnlyUrl = assetUrl;
            updateOnlySha256 = core::update::release_asset::digestSha256(asset);
        } else if (assetName.compare(QStringLiteral("sha256sums.txt"), Qt::CaseInsensitive) == 0) {
            checksumsUrl = assetUrl;
        }
    }

    const auto fullPkg = core::update::release_asset::resolveFullPackageUrl(assets, artifactLayout, releaseUrl);
    return core::update::UpdateInfo{
        currentVer, latestVersion, updateOnlyUrl, updateOnlySha256, checksumsUrl,
        fullPkg.url, releaseUrl, fullPkg.sha256, artifactLayout.guidance
    };
}

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
    request.setTransferTimeout(config::Update::kCheckTransferTimeoutMs);

    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            const QString error = reply->errorString();
            SPDLOG_ERROR("UpdateChecker: Network error checking for updates: {}", error.toStdString());
            emit checkFailed(error);
            return;
        }
        handleReleasesPayload(reply->readAll());
    });
}

void UpdateChecker::handleReleasesPayload(const QByteArray& rawData) {
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(rawData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
        const QString reason = (parseError.error != QJsonParseError::NoError)
            ? tr("Invalid release metadata JSON: %1").arg(parseError.errorString())
            : tr("Invalid release metadata format");
        SPDLOG_ERROR("UpdateChecker: Payload parse error: {}", reason.toStdString());
        emit checkFailed(reason);
        return;
    }

    const std::string jsonStr(rawData.constData(), rawData.size());
    const auto releases = core::update::release_parser::parseReleases(jsonStr, includePrereleaseOptIn());
    if (releases.empty()) {
        emit noUpdateAvailable(currentVersion());
        return;
    }

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

    const QString curVer = currentVersion();
    const int cmp = core::update::release_parser::compareVersions(latestRelease->tagName, curVer.toStdString());
    if (cmp > 0) {
        SPDLOG_INFO("UpdateChecker: New version available: v{} (Current: v{})",
                    latestVersion.toStdString(), curVer.toStdString());
        emit updateAvailable(buildUpdateInfo(*latestRelease, curVer, latestVersion, packagePlatform(), releasePageUrl()));
        return;
    }

    SPDLOG_INFO("UpdateChecker: No new version available (Current: v{})", curVer.toStdString());
    emit noUpdateAvailable(curVer);
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
