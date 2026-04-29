#include "application/AppLifecycleCoordinator.h"

#include "application/IMainWindowView.h"
#include "application/UpdateCoordinator.h"
#include "../core/common/SettingsController.h"
#include "../core/update/UpdateManager.h"

#include <QApplication>
#include <QtGlobal>

namespace ui::application {

AppLifecycleCoordinator::AppLifecycleCoordinator(IMainWindowView* view,
                                                 core::common::SettingsController* settingsController,
                                                 UpdateCoordinator* updateCoordinator)
    : view_(view),
      settingsController_(settingsController),
      updateCoordinator_(updateCoordinator) {
    Q_ASSERT(view_);
    Q_ASSERT(settingsController_);
}

void AppLifecycleCoordinator::restoreFromSettings() {
    view_->restoreWindowGeometry(settingsController_->mainWindowGeometry());
    view_->restoreWindowState(settingsController_->mainWindowState());
    view_->setNavigationCollapsed(settingsController_->navigationCollapsed());

    int timeoutMs = 0;
    int retries = 0;
    int retryIntervalMs = 0;
    bool retryEnabled = false;
    settingsController_->loadModbusSettings(timeoutMs, retries, retryIntervalMs, retryEnabled);
    view_->applyModbusSettings(timeoutMs, retryEnabled ? retries : 0, retryIntervalMs);
}

void AppLifecycleCoordinator::finalizeStartup() {
    core::update::UpdateManager::cleanupUpdateArtifacts();

    if (updateCoordinator_) {
        updateCoordinator_->refreshIndicators();
        updateCoordinator_->triggerAutoCheckIfNeeded();
    }

    if (!settingsController_->disclaimerAccepted()) {
        if (view_->showDisclaimerDialog()) {
            settingsController_->setDisclaimerAccepted(true);
        } else {
            qApp->quit();
        }
    }
}

void AppLifecycleCoordinator::persistOnClose() {
    settingsController_->setMainWindowGeometry(view_->saveWindowGeometry());
    settingsController_->setMainWindowState(view_->saveWindowState());
    settingsController_->sync();
}

} // namespace ui::application
