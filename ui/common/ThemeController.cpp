#include "ThemeController.h"
#include "SettingsKeys.h"

namespace ui::common {

ThemeController::ThemeController(ThemeRuntime& themeRuntime,
                                 ISettingsService& settingsService,
                                 QObject* parent)
    : QObject(parent),
      themeRuntime_(themeRuntime),
      settingsService_(settingsService),
      currentMode_(Theme::modeFromSetting(settingsService_.value(settings_keys::kAppThemeMode).toString())) {
    themeRuntime_.applyMode(currentMode_);

    connect(&themeRuntime_,
            &ThemeRuntime::systemThemeChanged,
            this,
            [this]() {
                if (currentMode_ == Theme::Mode::Auto) {
                    emit themeChanged();
                }
            });
}

Theme::Mode ThemeController::currentMode() const noexcept {
    return currentMode_;
}

void ThemeController::cycleMode() {
    Theme::Mode nextMode = Theme::Mode::Auto;
    switch (currentMode_) {
    case Theme::Mode::Auto:
        nextMode = Theme::Mode::Light;
        break;
    case Theme::Mode::Light:
        nextMode = Theme::Mode::Dark;
        break;
    case Theme::Mode::Dark:
        nextMode = Theme::Mode::Auto;
        break;
    }

    applyMode(nextMode, true);
}

void ThemeController::setMode(Theme::Mode mode) {
    applyMode(mode, true);
}

void ThemeController::applyMode(Theme::Mode mode, bool persist) {
    currentMode_ = mode;
    themeRuntime_.applyMode(currentMode_);

    if (persist) {
        settingsService_.setValue(settings_keys::kAppThemeMode, Theme::modeToSetting(currentMode_));
    }

    emit themeChanged();
}

} // namespace ui::common
