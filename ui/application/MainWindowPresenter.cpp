#include "application/MainWindowPresenter.h"

#include "application/IMainWindowView.h"
#include "application/UpdateCoordinator.h"
#include "../core/common/SettingsController.h"

#include <QtGlobal>

namespace ui::application {

MainWindowPresenter::MainWindowPresenter(IMainWindowView* view,
                                         core::common::SettingsController* settingsController,
                                         UpdateCoordinator* updateCoordinator)
    : view_(view),
      settingsController_(settingsController),
      updateCoordinator_(updateCoordinator) {
    Q_ASSERT(view_);
    Q_ASSERT(settingsController_);
}

void MainWindowPresenter::initialize() {
    view_->initializeUi();
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
    view_->applyLanguage(locale);
}

void MainWindowPresenter::onCloseRequested() {
    view_->persistWindowState();
}

} // namespace ui::application
