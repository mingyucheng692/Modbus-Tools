#include "application/UpdateCoordinator.h"

#include "AppConstants.h"
#include "common/UpdateChecker.h"
#include "../core/common/SettingsController.h"
#include "../core/update/UpdateManager.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <spdlog/spdlog.h>

namespace {

QString trMainWindow(const char* sourceText) {
    return QCoreApplication::translate("ui::MainWindow", sourceText);
}

} // namespace

namespace ui::application {

UpdateCoordinator::UpdateCoordinator(IUpdateInteractionView* view,
                                     common::UpdateChecker* updateChecker,
                                     core::update::UpdateManager* updateManager,
                                     core::common::SettingsController* settingsController,
                                     QObject* parent)
    : QObject(parent),
      view_(view),
      updateChecker_(updateChecker),
      updateManager_(updateManager),
      settingsController_(settingsController) {
    Q_ASSERT(view_);
    Q_ASSERT(updateChecker_);
    Q_ASSERT(updateManager_);
    Q_ASSERT(settingsController_);

    connect(updateChecker_, &common::UpdateChecker::updateAvailable,
            this, &UpdateCoordinator::handleUpdateAvailable);
    connect(updateChecker_, &common::UpdateChecker::noUpdateAvailable,
            this, &UpdateCoordinator::handleNoUpdateAvailable);
    connect(updateChecker_, &common::UpdateChecker::checkFailed,
            this, &UpdateCoordinator::handleCheckFailed);

    connect(updateManager_, &core::update::UpdateManager::updateReadyToInstall,
            this, &UpdateCoordinator::handleUpdateReadyToInstall);
    connect(updateManager_, &core::update::UpdateManager::updateFailed,
            this, &UpdateCoordinator::handleUpdateFailed);
}

void UpdateCoordinator::setCurrentLocale(const QString& locale) {
    currentLocale_ = locale;
}

void UpdateCoordinator::checkForUpdates() {
    if (updateAvailable_ && !pendingUpdateInfo_.latestVersion.isEmpty()) {
        promptUpdateAction(common::UpdateChecker::currentVersion());
        return;
    }

    if (view_) {
        view_->setUpdateCheckActionEnabled(false);
    }
    performUpdateCheck(true);
}

void UpdateCoordinator::triggerAutoCheckIfNeeded() {
    if (shouldAutoCheckUpdates()) {
        performUpdateCheck(false);
    }
}

void UpdateCoordinator::refreshIndicators() {
    if (view_) {
        view_->setUpdateIndicatorVisible(updateAvailable_);
    }
}

bool UpdateCoordinator::updateAvailable() const {
    return updateAvailable_;
}

void UpdateCoordinator::performUpdateCheck(bool manual) {
    checkingUpdateManually_ = manual;
    settingsController_->setLastUpdateCheckUtc(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    updateChecker_->checkForUpdates();
}

bool UpdateCoordinator::shouldAutoCheckUpdates() const {
    const QString freq = settingsController_->updateCheckFrequency();
    if (freq == app::constants::Values::App::kUpdateCheckNever) {
        spdlog::info("UpdateCoordinator: Auto update check skipped because frequency is set to 'never'.");
        return false;
    }
    if (freq == app::constants::Values::App::kUpdateCheckStartup) {
        return true;
    }

    const QDateTime lastCheck = QDateTime::fromString(settingsController_->lastUpdateCheckUtc(), Qt::ISODate);
    if (!lastCheck.isValid()) {
        return true;
    }

    const qint64 days = lastCheck.daysTo(QDateTime::currentDateTimeUtc());
    if (freq == app::constants::Values::App::kUpdateCheckWeekly) {
        if (days >= 7) {
            spdlog::info("UpdateCoordinator: Auto update check allowed for 'weekly' frequency ({} days since last check).", days);
        } else {
            spdlog::info("UpdateCoordinator: Auto update check skipped for 'weekly' frequency ({} days since last check, requires >= 7).", days);
        }
        return days >= 7;
    }
    if (freq == app::constants::Values::App::kUpdateCheckMonthly) {
        if (days >= 30) {
            spdlog::info("UpdateCoordinator: Auto update check allowed for 'monthly' frequency ({} days since last check).", days);
        } else {
            spdlog::info("UpdateCoordinator: Auto update check skipped for 'monthly' frequency ({} days since last check, requires >= 30).", days);
        }
        return days >= 30;
    }
    return true;
}

void UpdateCoordinator::handleUpdateAvailable(const QString& currentVersion,
                                              const QString& latestVersion,
                                              const QString& updateOnlyUrl,
                                              const QString& updateOnlySha256,
                                              const QString& checksumsUrl,
                                              const QString& fullPackageUrl,
                                              const QString& releaseUrl) {
    if (view_) {
        view_->setUpdateCheckActionEnabled(true);
    }

    updateAvailable_ = true;
    pendingUpdateInfo_ = {latestVersion, updateOnlyUrl, updateOnlySha256, checksumsUrl, fullPackageUrl, releaseUrl};
    refreshIndicators();

    if (checkingUpdateManually_) {
        promptUpdateAction(currentVersion);
    }
}

void UpdateCoordinator::handleNoUpdateAvailable(const QString& currentVersion) {
    if (view_) {
        view_->setUpdateCheckActionEnabled(true);
    }
    if (checkingUpdateManually_ && view_) {
        view_->showUpdateInfoMessage(
            trMainWindow("No Updates"),
            trMainWindow("You are using the latest version: v%1").arg(currentVersion));
    }
    checkingUpdateManually_ = false;
}

void UpdateCoordinator::handleCheckFailed(const QString& reason) {
    if (view_) {
        view_->setUpdateCheckActionEnabled(true);
    }
    if (checkingUpdateManually_ && view_) {
        view_->showUpdateWarningMessage(trMainWindow("Update Check Failed"), reason);
    }
    checkingUpdateManually_ = false;
}

void UpdateCoordinator::handleUpdateReadyToInstall(const QString& taskFile) {
    QString error;
    if (core::update::UpdateManager::launchUpdater(taskFile, currentLocale_, error)) {
        spdlog::info("UpdateCoordinator: Updater launched successfully, terminating application to apply update.");
        qApp->quit();
    } else {
        spdlog::error("UpdateCoordinator: Failed to launch updater: {}", error.toStdString());
        if (view_) {
            view_->showUpdateCriticalMessage(trMainWindow("Update Failed"), error);
        }
    }
    checkingUpdateManually_ = false;
}

void UpdateCoordinator::handleUpdateFailed(const QString& error) {
    if (checkingUpdateManually_ && view_) {
        view_->showUpdateWarningMessage(trMainWindow("Update Failed"), error);
    }
    checkingUpdateManually_ = false;
}

void UpdateCoordinator::promptUpdateAction(const QString& currentVersion) {
    const QString downloadUrl = pendingUpdateInfo_.fullPackageUrl.isEmpty()
        ? pendingUpdateInfo_.releaseUrl
        : pendingUpdateInfo_.fullPackageUrl;

    if (pendingUpdateInfo_.updateOnlyUrl.isEmpty()) {
        if (view_ && view_->confirmOpenDownloadPage(pendingUpdateInfo_.latestVersion)) {
            QDesktopServices::openUrl(QUrl(downloadUrl));
        }
        checkingUpdateManually_ = false;
        return;
    }

    if (!view_) {
        checkingUpdateManually_ = false;
        return;
    }

    switch (view_->promptUpdateAction(currentVersion, pendingUpdateInfo_.latestVersion)) {
    case UpdatePromptChoice::StartSilentUpdate:
        startSilentUpdate();
        break;
    case UpdatePromptChoice::DownloadFullPackage:
    case UpdatePromptChoice::OpenDownloadPage:
        QDesktopServices::openUrl(QUrl(downloadUrl));
        checkingUpdateManually_ = false;
        break;
    case UpdatePromptChoice::Cancel:
    default:
        checkingUpdateManually_ = false;
        break;
    }
}

void UpdateCoordinator::startSilentUpdate() {
    updateManager_->startUpdate(QUrl(pendingUpdateInfo_.updateOnlyUrl),
                                pendingUpdateInfo_.updateOnlySha256,
                                pendingUpdateInfo_.checksumsUrl,
                                pendingUpdateInfo_.latestVersion);

    if (view_) {
        view_->showUpdateProgress(updateManager_);
    }
}

} // namespace ui::application
