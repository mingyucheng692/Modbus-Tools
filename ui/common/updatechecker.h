#pragma once

#include <QObject>

class QNetworkAccessManager;
class QString;

namespace ui::common {

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit UpdateChecker(QObject* parent = nullptr);
    void checkForUpdates();

    static QString currentVersion();
    static QString packagePlatform();
    static QString releasePageUrl();
    static QString expectedZipName(const QString& version);

signals:
    void updateAvailable(const QString& currentVersion,
                         const QString& latestVersion,
                         const QString& downloadUrl,
                         const QString& releaseUrl);
    void noUpdateAvailable(const QString& currentVersion);
    void checkFailed(const QString& reason);

private:
    static QString normalizeVersion(const QString& rawVersion);
    static int compareVersions(const QString& left, const QString& right);

    QNetworkAccessManager* networkManager_ = nullptr;
};

} // namespace ui::common
