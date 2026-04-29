#include "application/MainWindowPresenter.h"

#include "application/IMainWindowView.h"
#include "application/LanguageCoordinator.h"
#include "application/UpdateCoordinator.h"
#include "../core/common/SettingsController.h"

#include <QtGlobal>

namespace ui::application {

MainWindowPresenter::MainWindowPresenter(IMainWindowView* view,
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

void MainWindowPresenter::initialize() {
    view_->initializeUi();
    languageCoordinator_->initialize();
    if (updateCoordinator_) {
        updateCoordinator_->setCurrentLocale(languageCoordinator_->currentLocale());
    }
    view_->retranslateUi(languageCoordinator_->effectiveLocale());
}

void MainWindowPresenter::onNavigationToggleRequested() {
    const bool nextCollapsed = !view_->isNavigationCollapsed();
    view_->setNavigationCollapsed(nextCollapsed);
    settingsController_->setNavigationCollapsed(nextCollapsed);
}

void MainWindowPresenter::onModbusSettingsRequested() {
    view_->openModbusSettingsDialog();
}

void MainWindowPresenter::onUpdateSettingsRequested() {
    view_->openUpdateSettingsDialog();
}

void MainWindowPresenter::onAboutRequested() {
    view_->openAboutDialog();
}

void MainWindowPresenter::onCheckForUpdatesRequested() {
    if (updateCoordinator_) {
        updateCoordinator_->checkForUpdates();
    }
}

void MainWindowPresenter::onLanguageSelected(const QString& locale) {
    languageCoordinator_->applyLanguage(locale);
    if (updateCoordinator_) {
        updateCoordinator_->setCurrentLocale(languageCoordinator_->currentLocale());
    }
    view_->retranslateUi(languageCoordinator_->effectiveLocale());
}

void MainWindowPresenter::onCloseRequested() {
    view_->persistWindowState();
}

} // namespace ui::application
