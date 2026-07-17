#include "application/AppLifecycleCoordinator.h"

#include "application/IMainWindowView.h"
#include "application/LanguageCoordinator.h"
#include "application/UpdateCoordinator.h"
#include "../core/common/SettingsController.h"
#include "../core/update/UpdateManager.h"

#include <QApplication>
#include <QtGlobal>

namespace ui::application {

AppLifecycleCoordinator::AppLifecycleCoordinator(IMainWindowView* view,
                                                 core::common::SettingsController* settingsController,
                                                 LanguageCoordinator* languageCoordinator,
                                                 UpdateCoordinator* updateCoordinator)
    : view_(view),
      settingsController_(settingsController),
      languageCoordinator_(languageCoordinator),
      updateCoordinator_(updateCoordinator) {
    Q_ASSERT(view_);
    Q_ASSERT(settingsController_);
    Q_ASSERT(languageCoordinator_);
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
            view_->requestQuit();
        }
    }
}

void AppLifecycleCoordinator::persistOnClose() {
    settingsController_->setMainWindowGeometry(view_->saveWindowGeometry());
    settingsController_->setMainWindowState(view_->saveWindowState());
    settingsController_->sync();
}

void AppLifecycleCoordinator::initialize() {
    view_->initializeUi();
    restoreFromSettings();
    languageCoordinator_->initialize();
    if (updateCoordinator_) {
        updateCoordinator_->setCurrentLocale(languageCoordinator_->currentLocale());
    }
    view_->retranslateUi(languageCoordinator_->effectiveLocale());
    finalizeStartup();
}

void AppLifecycleCoordinator::onNavigationToggleRequested() {
    const bool nextCollapsed = !view_->isNavigationCollapsed();
    view_->setNavigationCollapsed(nextCollapsed);
    settingsController_->setNavigationCollapsed(nextCollapsed);
}

void AppLifecycleCoordinator::onModbusSettingsRequested() {
    view_->openModbusSettingsDialog();
}

void AppLifecycleCoordinator::onUpdateSettingsRequested() {
    view_->openUpdateSettingsDialog();
}

void AppLifecycleCoordinator::onAboutRequested() {
    view_->openAboutDialog();
}

void AppLifecycleCoordinator::onCheckForUpdatesRequested() {
    if (updateCoordinator_) {
        updateCoordinator_->checkForUpdates();
    }
}

void AppLifecycleCoordinator::onLanguageSelected(const QString& locale) {
    languageCoordinator_->applyLanguage(locale);
    if (updateCoordinator_) {
        updateCoordinator_->setCurrentLocale(languageCoordinator_->currentLocale());
    }
    view_->retranslateUi(languageCoordinator_->effectiveLocale());
}

void AppLifecycleCoordinator::onCloseRequested() {
    persistOnClose();
}

} // namespace ui::application
