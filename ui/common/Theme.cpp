/**
 * @file Theme.cpp
 * @brief Implementation of Theme.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "Theme.h"
namespace ui::common {

Theme::Mode Theme::modeFromSetting(const QString& value) {
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("light")) {
        return Mode::Light;
    }
    if (normalized == QStringLiteral("dark")) {
        return Mode::Dark;
    }
    return Mode::Auto;
}

QString Theme::modeToSetting(Mode mode) {
    switch (mode) {
    case Mode::Auto:
        return QStringLiteral("auto");
    case Mode::Light:
        return QStringLiteral("light");
    case Mode::Dark:
        return QStringLiteral("dark");
    }
    return QStringLiteral("auto");
}

Qt::ColorScheme Theme::colorSchemeForMode(Mode mode) {
    switch (mode) {
    case Mode::Auto:
        return Qt::ColorScheme::Unknown;
    case Mode::Light:
        return Qt::ColorScheme::Light;
    case Mode::Dark:
        return Qt::ColorScheme::Dark;
    }
    return Qt::ColorScheme::Unknown;
}

} // namespace ui::common
