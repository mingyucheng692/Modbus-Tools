#pragma once

#include "application/IMainWindowPresenter.h"
#include <QString>

namespace core::common {
class SettingsController;
}

namespace ui::application {

class AppLifecycleCoordinator;
class IMainWindowView;
class LanguageCoordinator;
class UpdateCoordinator;

class MainWindowPresenter final : public IMainWindowPresenter {
public:
    MainWindowPresenter(IMainWindowView* view,
                        core::common::SettingsController* settingsController,
                        AppLifecycleCoordinator* appLifecycleCoordinator,
                        LanguageCoordinator* languageCoordinator,
                        UpdateCoordinator* updateCoordinator);

    void initialize() override;
    void onNavigationToggleRequested() override;
    void onModbusSettingsRequested() override;
    void onUpdateSettingsRequested() override;
    void onAboutRequested() override;
    void onCheckForUpdatesRequested() override;
    void onLanguageSelected(const QString& locale) override;
    void onCloseRequested() override;

private:
    IMainWindowView* view_ = nullptr;
    core::common::SettingsController* settingsController_ = nullptr;
    AppLifecycleCoordinator* appLifecycleCoordinator_ = nullptr;
    LanguageCoordinator* languageCoordinator_ = nullptr;
    UpdateCoordinator* updateCoordinator_ = nullptr;
};

} // namespace ui::application
