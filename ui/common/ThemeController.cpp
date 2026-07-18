/**
 * @file ThemeController.cpp
 * @brief Implementation of ThemeController.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ThemeController.h"
#include "common/SettingsKeys.h"

#include <QApplication>
#include <QFrame>
#include <QGuiApplication>
#include <QIcon>
#include <QImage>
#include <QListWidget>
#include <QMenuBar>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QStyle>
#include <QStyleHints>
#include <QToolButton>
#include <QWidget>

#include <array>

namespace ui::common {

// --- Constructor & state management (formerly ThemeController.cpp + QtThemeRuntime.cpp) ---

ThemeController::ThemeController(::core::common::ISettingsService& settingsService, QObject* parent)
    : QObject(parent),
      settingsService_(settingsService),
      currentMode_(Theme::modeFromSetting(settingsService_.value(core::common::settings_keys::kAppThemeMode).toString())) {
    QGuiApplication::styleHints()->setColorScheme(Theme::colorSchemeForMode(currentMode_));

    connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, [this](Qt::ColorScheme) {
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
    QGuiApplication::styleHints()->setColorScheme(Theme::colorSchemeForMode(currentMode_));

    if (persist) {
        settingsService_.setValue(core::common::settings_keys::kAppThemeMode, Theme::modeToSetting(currentMode_));
    }

    emit themeChanged();
}

} // namespace ui::common
