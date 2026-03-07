#include "UpdateChecker.h"

#include <QJsonArray>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QUrl>

namespace {

#ifndef MODBUS_TOOLS_APP_VERSION
#error "MODBUS_TOOLS_APP_VERSION must be defined by CMake"
#endif

#ifndef MODBUS_TOOLS_RELEASES_API_URL
#define MODBUS_TOOLS_RELEASES_API_URL "https://api.github.com/repos/mingyucheng692/Modbus-Tools/releases/latest"
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
    QNetworkRequest request(QUrl(QStringLiteral(MODBUS_TOOLS_RELEASES_API_URL)));
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Modbus-Tools/%1").arg(currentVersion()));
    request.setRawHeader("Accept", "application/vnd.github+json");

    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            emit checkFailed(reply->errorString());
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
        if (!document.isObject()) {
            emit checkFailed(tr("Invalid release response"));
            return;
        }

        const QJsonObject root = document.object();
        const QString latestVersion = normalizeVersion(root.value("tag_name").toString());
        if (latestVersion.isEmpty()) {
            emit checkFailed(tr("Release tag is missing"));
            return;
        }

        QString releaseUrl = root.value("html_url").toString();
        if (releaseUrl.isEmpty()) {
            releaseUrl = releasePageUrl();
        }

        QString downloadUrl;
        const QString expectedAsset = expectedZipName(latestVersion);
        const QJsonArray assets = root.value("assets").toArray();
        for (const QJsonValue& assetValue : assets) {
            if (!assetValue.isObject()) {
                continue;
            }
            const QJsonObject asset = assetValue.toObject();
            const QString assetName = asset.value("name").toString();
            if (assetName.compare(expectedAsset, Qt::CaseInsensitive) == 0) {
                downloadUrl = asset.value("browser_download_url").toString();
                break;
            }
        }

        if (downloadUrl.isEmpty() && !assets.isEmpty() && assets.first().isObject()) {
            downloadUrl = assets.first().toObject().value("browser_download_url").toString();
        }
        if (downloadUrl.isEmpty()) {
            downloadUrl = releaseUrl;
        }

        const int compareResult = compareVersions(latestVersion, currentVersion());
        if (compareResult > 0) {
            emit updateAvailable(currentVersion(), latestVersion, downloadUrl, releaseUrl);
            return;
        }

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
