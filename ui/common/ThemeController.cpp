#include "ThemeController.h"
#include "SettingsKeys.h"
#include <QApplication>
#include <QStyleHints>

namespace ui::common {

ThemeController::ThemeController(QApplication& application,
                                 ISettingsService& settingsService,
                                 QObject* parent)
    : QObject(parent),
      application_(application),
      settingsService_(settingsService),
      currentMode_(Theme::modeFromSetting(settingsService_.value(settings_keys::kAppThemeMode).toString())) {
    Theme::apply(application_, currentMode_);

    connect(application_.styleHints(),
            &QStyleHints::colorSchemeChanged,
            this,
            [this](Qt::ColorScheme colorScheme) {
                Q_UNUSED(colorScheme);
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
    Theme::apply(application_, currentMode_);

    if (persist) {
        settingsService_.setValue(settings_keys::kAppThemeMode, Theme::modeToSetting(currentMode_));
    }

    emit themeChanged();
}

} // namespace ui::common
