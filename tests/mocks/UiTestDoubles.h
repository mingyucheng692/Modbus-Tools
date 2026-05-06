#pragma once

#include "../../ui/application/IMainWindowView.h"
#include "../../ui/application/UpdateCoordinator.h"
#include "../../core/update/UpdateManager.h"
#include "../../ui/common/ISettingsService.h"
#include "../../ui/common/UpdateChecker.h"

#include <QHash>
#include <QString>
#include <QVariant>

namespace tests::mocks {

class FakeSettingsService final : public ui::common::ISettingsService {
public:
    QVariant value(const QString& key) const override { return values_.value(key); }
    bool contains(const QString& key) const override { return values_.contains(key); }
    void setValue(const QString& key, const QVariant& value) override { values_.insert(key, value); }
    QString configFilePath() const override { return QStringLiteral("test-config.ini"); }
    void sync() override { ++syncCount; }

    QHash<QString, QVariant> values_;
    int syncCount = 0;
};

class FakeMainWindowView final : public ui::application::IMainWindowView,
                                 public ui::application::IUpdateInteractionView {
public:
    void initializeUi() override { ++initializeUiCallCount; }
    void setNavigationCollapsed(bool collapsed) override { navigationCollapsed = collapsed; }
    [[nodiscard]] bool isNavigationCollapsed() const override { return navigationCollapsed; }
    void restoreWindowGeometry(const QByteArray& geometry) override {
        restoredGeometry = geometry;
        ++restoreWindowGeometryCallCount;
    }
    void restoreWindowState(const QByteArray& state) override {
        restoredState = state;
        ++restoreWindowStateCallCount;
    }
    [[nodiscard]] QByteArray saveWindowGeometry() const override { return savedGeometry; }
    [[nodiscard]] QByteArray saveWindowState() const override { return savedState; }
    void applyModbusSettings(int timeoutMs, int retries, int retryIntervalMs) override {
        appliedTimeoutMs = timeoutMs;
        appliedRetries = retries;
        appliedRetryIntervalMs = retryIntervalMs;
        ++applyModbusSettingsCallCount;
    }
    void openModbusSettingsDialog() override { ++openModbusSettingsDialogCallCount; }
    void openUpdateSettingsDialog() override { ++openUpdateSettingsDialogCallCount; }
    void openAboutDialog() override { ++openAboutDialogCallCount; }
    [[nodiscard]] bool showDisclaimerDialog() override {
        ++showDisclaimerDialogCallCount;
        return disclaimerDialogResult;
    }
    void retranslateUi(const QString& effectiveLocale) override {
        lastRetranslatedLocale = effectiveLocale;
        ++retranslateUiCallCount;
    }

    void setUpdateCheckActionEnabled(bool enabled) override {
        updateCheckActionEnabled = enabled;
        ++setUpdateCheckActionEnabledCallCount;
    }
    void setUpdateIndicatorVisible(bool visible) override {
        updateIndicatorVisible = visible;
        ++setUpdateIndicatorVisibleCallCount;
    }
    void showUpdateInfoMessage(const QString& title, const QString& message) override {
        lastInfoTitle = title;
        lastInfoMessage = message;
        ++showUpdateInfoMessageCallCount;
    }
    void showUpdateWarningMessage(const QString& title, const QString& message) override {
        lastWarningTitle = title;
        lastWarningMessage = message;
        ++showUpdateWarningMessageCallCount;
    }
    void showUpdateCriticalMessage(const QString& title, const QString& message) override {
        lastCriticalTitle = title;
        lastCriticalMessage = message;
        ++showUpdateCriticalMessageCallCount;
    }
    [[nodiscard]] bool confirmOpenDownloadPage(const QString& latestVersion) override {
        lastConfirmedLatestVersion = latestVersion;
        ++confirmOpenDownloadPageCallCount;
        return confirmOpenDownloadPageResult;
    }
    [[nodiscard]] ui::application::UpdatePromptChoice promptUpdateAction(const QString& currentVersion,
                                                                         const QString& latestVersion) override {
        lastPromptCurrentVersion = currentVersion;
        lastPromptLatestVersion = latestVersion;
        ++promptUpdateActionCallCount;
        return promptChoice;
    }
    void showUpdateProgress(core::update::UpdateManager* updateManager) override {
        lastProgressUpdateManager = updateManager;
        ++showUpdateProgressCallCount;
    }

    bool navigationCollapsed = false;
    QByteArray restoredGeometry;
    QByteArray restoredState;
    QByteArray savedGeometry = QByteArrayLiteral("saved-geometry");
    QByteArray savedState = QByteArrayLiteral("saved-state");
    int appliedTimeoutMs = 0;
    int appliedRetries = 0;
    int appliedRetryIntervalMs = 0;
    bool disclaimerDialogResult = true;
    QString lastRetranslatedLocale;
    bool updateCheckActionEnabled = true;
    bool updateIndicatorVisible = false;
    bool confirmOpenDownloadPageResult = false;
    ui::application::UpdatePromptChoice promptChoice = ui::application::UpdatePromptChoice::Cancel;
    core::update::UpdateManager* lastProgressUpdateManager = nullptr;
    QString lastInfoTitle;
    QString lastInfoMessage;
    QString lastWarningTitle;
    QString lastWarningMessage;
    QString lastCriticalTitle;
    QString lastCriticalMessage;
    QString lastConfirmedLatestVersion;
    QString lastPromptCurrentVersion;
    QString lastPromptLatestVersion;
    int initializeUiCallCount = 0;
    int restoreWindowGeometryCallCount = 0;
    int restoreWindowStateCallCount = 0;
    int applyModbusSettingsCallCount = 0;
    int openModbusSettingsDialogCallCount = 0;
    int openUpdateSettingsDialogCallCount = 0;
    int openAboutDialogCallCount = 0;
    int showDisclaimerDialogCallCount = 0;
    int retranslateUiCallCount = 0;
    int setUpdateCheckActionEnabledCallCount = 0;
    int setUpdateIndicatorVisibleCallCount = 0;
    int showUpdateInfoMessageCallCount = 0;
    int showUpdateWarningMessageCallCount = 0;
    int showUpdateCriticalMessageCallCount = 0;
    int confirmOpenDownloadPageCallCount = 0;
    int promptUpdateActionCallCount = 0;
    int showUpdateProgressCallCount = 0;
};

class FakeUpdateChecker : public ui::common::UpdateChecker {
public:
    explicit FakeUpdateChecker(QObject* parent = nullptr) : ui::common::UpdateChecker(parent) {}

    void checkForUpdates() override { ++checkForUpdatesCallCount; }

    void emitUpdateAvailable(const QString& currentVersion,
                             const QString& latestVersion,
                             const QString& updateOnlyUrl,
                             const QString& updateOnlySha256,
                             const QString& checksumsUrl,
                             const QString& fullPackageUrl,
                             const QString& releaseUrl) {
        emit updateAvailable(currentVersion,
                             latestVersion,
                             updateOnlyUrl,
                             updateOnlySha256,
                             checksumsUrl,
                             fullPackageUrl,
                             releaseUrl);
    }

    void emitNoUpdateAvailable(const QString& currentVersion) {
        emit noUpdateAvailable(currentVersion);
    }

    void emitCheckFailed(const QString& reason) {
        emit checkFailed(reason);
    }

    int checkForUpdatesCallCount = 0;
};

class FakeUpdateManager : public core::update::UpdateManager {
public:
    explicit FakeUpdateManager(QObject* parent = nullptr) : core::update::UpdateManager(parent) {}

    void emitUpdateReadyToInstall(const QString& taskFilePath) {
        emit updateReadyToInstall(taskFilePath);
    }

    void emitUpdateFailed(const QString& reason) {
        emit updateFailed(reason);
    }

    void emitUpdateCanceled() {
        emit updateCanceled();
    }
};

} // namespace tests::mocks
