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
struct ReleaseInfo;
struct UpdateInfo;
}

namespace ui::common {

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit UpdateChecker(QObject* parent = nullptr);
    void checkForUpdates();

    static QString currentVersion();
    static QString packagePlatform();
    static QString releasePageUrl();

signals:
    void updateAvailable(const core::update::UpdateInfo& info);
    void noUpdateAvailable(const QString& currentVersion);
    void checkFailed(const QString& reason);

private:
    QNetworkAccessManager* networkManager_ = nullptr;
};

} // namespace ui::common
