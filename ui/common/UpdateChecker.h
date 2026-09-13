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
    /// Virtual test seam: unit tests substitute FakeUpdateChecker to
    /// intercept checks without touching the network.
    virtual void checkForUpdates();

    static QString currentVersion();
    static QString packagePlatform();
    static QString releasePageUrl();

    /// Runtime opt-in gate for the prerelease update channel: re-reads the
    /// MODBUS_TOOLS_PRERELEASE environment variable on every check. Returns
    /// true only when the variable is set to "1" or "true" (case-insensitive);
    /// unset, empty, "0" and "false" all resolve to false. This replaces the
    /// retired MODBUS_TOOLS_INCLUDE_PRERELEASE compile-time option, which
    /// permanently burned the channel choice into the distributed binary.
    static bool includePrereleaseOptIn();

signals:
    void updateAvailable(const core::update::UpdateInfo& info);
    void noUpdateAvailable(const QString& currentVersion);
    void checkFailed(const QString& reason);

private:
    QNetworkAccessManager* networkManager_ = nullptr;
};

} // namespace ui::common
