#pragma once

#include <QObject>
#include <QString>

class QUrl;

namespace core::common {
class SettingsController;
}

namespace core::update {
class UpdateManager;
}

namespace ui::common {
class UpdateChecker;
}

namespace ui::application {

enum class UpdatePromptChoice {
    Cancel,
    StartSilentUpdate,
    DownloadFullPackage,
    OpenDownloadPage
};

class IUpdateInteractionView {
public:
    virtual ~IUpdateInteractionView() = default;

    virtual void setUpdateCheckActionEnabled(bool enabled) = 0;
    virtual void setUpdateIndicatorVisible(bool visible) = 0;
    virtual void showUpdateInfoMessage(const QString& title, const QString& message) = 0;
    virtual void showUpdateWarningMessage(const QString& title, const QString& message) = 0;
    virtual void showUpdateCriticalMessage(const QString& title, const QString& message) = 0;
    virtual bool confirmOpenDownloadPage(const QString& latestVersion) = 0;
    virtual UpdatePromptChoice promptUpdateAction(const QString& currentVersion, const QString& latestVersion) = 0;
    virtual void showUpdateProgress(core::update::UpdateManager* updateManager) = 0;
};

class UpdateCoordinator : public QObject {
    Q_OBJECT

public:
    explicit UpdateCoordinator(IUpdateInteractionView* view,
                               common::UpdateChecker* updateChecker,
                               core::update::UpdateManager* updateManager,
                               core::common::SettingsController* settingsController,
                               QObject* parent = nullptr);

    virtual void setCurrentLocale(const QString& locale);
    virtual void checkForUpdates();
    virtual void triggerAutoCheckIfNeeded();
    virtual void refreshIndicators();

    [[nodiscard]] bool updateAvailable() const;

private:
    struct PendingUpdateInfo {
        QString latestVersion;
        QString updateOnlyUrl;
        QString updateOnlySha256;
        QString checksumsUrl;
        QString fullPackageUrl;
        QString releaseUrl;
    };

    void performUpdateCheck(bool manual);
    bool shouldAutoCheckUpdates() const;
    void handleUpdateAvailable(const QString& currentVersion,
                               const QString& latestVersion,
                               const QString& updateOnlyUrl,
                               const QString& updateOnlySha256,
                               const QString& checksumsUrl,
                               const QString& fullPackageUrl,
                               const QString& releaseUrl);
    void handleNoUpdateAvailable(const QString& currentVersion);
    void handleCheckFailed(const QString& reason);
    void handleUpdateReadyToInstall(const QString& taskFile);
    void handleUpdateFailed(const QString& error);
    void promptUpdateAction(const QString& currentVersion);
    void startSilentUpdate();

    IUpdateInteractionView* view_ = nullptr;
    common::UpdateChecker* updateChecker_ = nullptr;
    core::update::UpdateManager* updateManager_ = nullptr;
    core::common::SettingsController* settingsController_ = nullptr;
    QString currentLocale_ = QStringLiteral("en_US");
    bool updateAvailable_ = false;
    bool checkingUpdateManually_ = false;
    PendingUpdateInfo pendingUpdateInfo_;
};

} // namespace ui::application
