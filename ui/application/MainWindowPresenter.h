#pragma once

#include <QString>

namespace core::common {
class SettingsController;
}

namespace ui::application {

class IMainWindowView;
class UpdateCoordinator;

class MainWindowPresenter final {
public:
    MainWindowPresenter(IMainWindowView* view,
                        core::common::SettingsController* settingsController,
                        UpdateCoordinator* updateCoordinator);

    void initialize();
    void onNavigationToggleRequested();
    void onModbusSettingsRequested();
    void onUpdateSettingsRequested();
    void onAboutRequested();
    void onCheckForUpdatesRequested();
    void onLanguageSelected(const QString& locale);
    void onCloseRequested();

private:
    IMainWindowView* view_ = nullptr;
    core::common::SettingsController* settingsController_ = nullptr;
    UpdateCoordinator* updateCoordinator_ = nullptr;
};

} // namespace ui::application
