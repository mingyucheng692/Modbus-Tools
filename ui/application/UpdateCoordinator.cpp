#include "application/UpdateCoordinator.h"

#include "Config.h"
#include "common/UpdateChecker.h"
#include "../core/common/SettingsController.h"
#include "../core/update/UpdateManager.h"
#include "infra/platform/PlatformInfo.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <spdlog/spdlog.h>
#include "../core/common/TrContext.h"

namespace {

constexpr char kMainWindowCtx[] = "ui::MainWindow";

} // namespace

namespace ui::application {

UpdateCoordinator::UpdateCoordinator(IUpdateInteractionView* view,
                                     std::function<void()> requestQuit,
                                     common::UpdateChecker* updateChecker,
                                     core::update::UpdateManager* updateManager,
                                     core::common::SettingsController* settingsController,
                                     QObject* parent)
    : QObject(parent),
      view_(view),
      requestQuit_(std::move(requestQuit)),
      updateChecker_(updateChecker),
      updateManager_(updateManager),
      settingsController_(settingsController) {
    Q_ASSERT(view_);
    Q_ASSERT(requestQuit_);
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
    connect(updateManager_, &core::update::UpdateManager::downloadProgress,
            this, [this](int progress) {
                if (view_) view_->updateProgress(progress, {});
            });
    connect(updateManager_, &core::update::UpdateManager::updateCanceled,
            this, [this] {
                if (view_) view_->hideUpdateProgress();
            });
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
    if (freq == config::App::kUpdateCheckNever) {
        SPDLOG_INFO("UpdateCoordinator: Auto update check skipped because frequency is set to 'never'.");
        return false;
    }
    if (freq == config::App::kUpdateCheckStartup) {
        return true;
    }

    const QDateTime lastCheck = QDateTime::fromString(settingsController_->lastUpdateCheckUtc(), Qt::ISODate);
    if (!lastCheck.isValid()) {
        return true;
    }

    const qint64 days = lastCheck.daysTo(QDateTime::currentDateTimeUtc());
    if (freq == config::App::kUpdateCheckWeekly) {
        if (days >= 7) {
            SPDLOG_INFO("UpdateCoordinator: Auto update check allowed for 'weekly' frequency ({} days since last check).", days);
        } else {
            SPDLOG_INFO("UpdateCoordinator: Auto update check skipped for 'weekly' frequency ({} days since last check, requires >= 7).", days);
        }
        return days >= 7;
    }
    if (freq == config::App::kUpdateCheckMonthly) {
        if (days >= 30) {
            SPDLOG_INFO("UpdateCoordinator: Auto update check allowed for 'monthly' frequency ({} days since last check).", days);
        } else {
            SPDLOG_INFO("UpdateCoordinator: Auto update check skipped for 'monthly' frequency ({} days since last check, requires >= 30).", days);
        }
        return days >= 30;
    }
    return true;
}

void UpdateCoordinator::handleUpdateAvailable(const core::update::UpdateInfo& info) {
    if (view_) {
        view_->setUpdateCheckActionEnabled(true);
    }

    updateAvailable_ = true;
    pendingUpdateInfo_ = info;
    refreshIndicators();

    if (checkingUpdateManually_) {
        promptUpdateAction(info.currentVersion);
    }
}

void UpdateCoordinator::handleNoUpdateAvailable(const QString& currentVersion) {
    if (view_) {
        view_->setUpdateCheckActionEnabled(true);
    }
    if (checkingUpdateManually_ && view_) {
        view_->showUpdateInfoMessage(
            TrContext<kMainWindowCtx>::tr("No Updates"),
            TrContext<kMainWindowCtx>::tr("You are using the latest version: v%1").arg(currentVersion));
    }
    checkingUpdateManually_ = false;
}

void UpdateCoordinator::handleCheckFailed(const QString& reason) {
    if (view_) {
        view_->setUpdateCheckActionEnabled(true);
    }
    if (checkingUpdateManually_ && view_) {
        view_->showUpdateWarningMessage(TrContext<kMainWindowCtx>::tr("Update Check Failed"), reason);
    }
    checkingUpdateManually_ = false;
}

void UpdateCoordinator::handleUpdateReadyToInstall(const QString& taskFile) {
    if (view_) view_->hideUpdateProgress();
    QString error;
    if (updateManager_ != nullptr && updateManager_->launchInstaller(taskFile, currentLocale_, error)) {
        SPDLOG_INFO("UpdateCoordinator: Updater launched successfully, terminating application to apply update.");
        if (requestQuit_) {
            requestQuit_();
        }
    } else {
        if (updateManager_ == nullptr && error.isEmpty()) {
            error = TrContext<kMainWindowCtx>::tr("Update service unavailable");
        }
        SPDLOG_ERROR("UpdateCoordinator: Failed to launch updater: {}", error.toStdString());
        if (view_) {
            view_->showUpdateCriticalMessage(TrContext<kMainWindowCtx>::tr("Update Failed"), error);
        }
    }
    checkingUpdateManually_ = false;
}

void UpdateCoordinator::handleUpdateFailed(const QString& error) {
    if (view_) view_->hideUpdateProgress();
    if (checkingUpdateManually_ && view_) {
        view_->showUpdateWarningMessage(TrContext<kMainWindowCtx>::tr("Update Failed"), error);
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

    if (updateManager_ == nullptr ||
        updateManager_->installMode() == core::update::UpdateInstallMode::DownloadOnly) {
        if (view_) {
            view_->showUpdateInfoMessage(
                TrContext<kMainWindowCtx>::tr("Automatic Update Unsupported"),
                TrContext<kMainWindowCtx>::tr("In-app automatic update is not supported on %1. Download the latest package instead.")
                    .arg(infra::platform::platformDisplayName()));
            if (view_->confirmOpenDownloadPage(pendingUpdateInfo_.latestVersion)) {
                QDesktopServices::openUrl(QUrl(downloadUrl));
            }
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
        view_->showUpdateProgress([this] { updateManager_->cancelUpdate(); });
    }
}

} // namespace ui::application
