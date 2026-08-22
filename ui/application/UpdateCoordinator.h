#pragma once

#include <QObject>
#include <QString>
#include <functional>
#include "update/UpdateManager.h"

class QUrl;

namespace core::common {
class SettingsController;
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
    virtual void showUpdateProgress(std::function<void()> onCancel) = 0;
    virtual void updateProgress(int percent, const QString& message) = 0;
    virtual void hideUpdateProgress() = 0;
};

class UpdateCoordinator : public QObject {
    Q_OBJECT

public:
    /// @param requestQuit Replaces the former IApplicationExitView*
    ///        single-method interface. Invoked on the GUI thread after
    ///        the updater was launched successfully. The composition root must
    ///        guarantee the captured target outlives this coordinator.
    explicit UpdateCoordinator(IUpdateInteractionView* view,
                               std::function<void()> requestQuit,
                               common::UpdateChecker* updateChecker,
                               core::update::UpdateManager* updateManager,
                               core::common::SettingsController* settingsController,
                               QObject* parent = nullptr);

    /// Virtual test seam: AppLifecycleCoordinatorTest mocks these entry
    /// points to verify delegation without driving real update flows.
    virtual void setCurrentLocale(const QString& locale);
    virtual void checkForUpdates();
    virtual void triggerAutoCheckIfNeeded();
    virtual void refreshIndicators();

    [[nodiscard]] bool updateAvailable() const;

private:
    void performUpdateCheck(bool manual);
    bool shouldAutoCheckUpdates() const;
    void handleUpdateAvailable(const core::update::UpdateInfo& info);
    void handleNoUpdateAvailable(const QString& currentVersion);
    void handleCheckFailed(const QString& reason);
    void handleUpdateReadyToInstall(const QString& taskFile);
    void handleUpdateFailed(const QString& error);
    void promptUpdateAction(const QString& currentVersion);
    void startSilentUpdate();

    IUpdateInteractionView* view_ = nullptr;
    std::function<void()> requestQuit_;
    common::UpdateChecker* updateChecker_ = nullptr;
    core::update::UpdateManager* updateManager_ = nullptr;
    core::common::SettingsController* settingsController_ = nullptr;
    QString currentLocale_ = QStringLiteral("en_US");
    bool updateAvailable_ = false;
    bool checkingUpdateManually_ = false;
    core::update::UpdateInfo pendingUpdateInfo_;
};

} // namespace ui::application
