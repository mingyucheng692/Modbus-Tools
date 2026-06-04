/**
 * @file UpdateChecker.h
 * @brief Header file for UpdateChecker.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>

class QNetworkAccessManager;
class QJsonArray;
class QString;

namespace core::update {
struct PlatformUpdateArtifactLayout;
}

namespace ui::common {

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit UpdateChecker(QObject* parent = nullptr);
    virtual void checkForUpdates();

    static QString currentVersion();
    static QString packagePlatform();
    static QString releasePageUrl();
    static core::update::PlatformUpdateArtifactLayout currentArtifactLayout(const QString& version);
    static QString resolveFullPackageUrl(const QJsonArray& assets,
                                         const core::update::PlatformUpdateArtifactLayout& layout,
                                         const QString& releaseUrl);

signals:
    void updateAvailable(const QString& currentVersion,
                         const QString& latestVersion,
                         const QString& updateOnlyUrl,
                         const QString& updateOnlySha256,
                         const QString& checksumsUrl,
                         const QString& fullPackageUrl,
                         const QString& releaseUrl);
    void noUpdateAvailable(const QString& currentVersion);
    void checkFailed(const QString& reason);

private:
    static QString normalizeVersion(const QString& rawVersion);
    static int compareVersions(const QString& left, const QString& right);

    QNetworkAccessManager* networkManager_ = nullptr;
};

} // namespace ui::common
