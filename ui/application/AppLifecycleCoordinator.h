#pragma once

#include <QByteArray>

namespace core::common {
class SettingsController;
}

namespace ui::application {

class IMainWindowView;
class UpdateCoordinator;

class AppLifecycleCoordinator final {
public:
    AppLifecycleCoordinator(IMainWindowView* view,
                            core::common::SettingsController* settingsController,
                            UpdateCoordinator* updateCoordinator);

    void restoreFromSettings();
    void finalizeStartup();
    void persistOnClose();

private:
    IMainWindowView* view_ = nullptr;
    core::common::SettingsController* settingsController_ = nullptr;
    UpdateCoordinator* updateCoordinator_ = nullptr;
};

} // namespace ui::application
