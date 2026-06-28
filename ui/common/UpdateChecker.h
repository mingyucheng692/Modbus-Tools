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
#include <QJsonArray>

class QNetworkAccessManager;
class QString;

namespace core::update {
struct PlatformUpdateArtifactLayout;
struct ReleaseInfo;
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
    QNetworkAccessManager* networkManager_ = nullptr;
};

} // namespace ui::common
