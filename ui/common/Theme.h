/**
 * @file Theme.h
 * @brief Header file for Theme.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QtCore/Qt>
#include <QString>

namespace ui::common {

class Theme {
public:
    enum class Mode {
        Auto,
        Light,
        Dark
    };

    [[nodiscard]] static Mode modeFromSetting(const QString& value) {
        const QString normalized = value.trimmed().toLower();
        if (normalized == QStringLiteral("light")) {
            return Mode::Light;
        }
        if (normalized == QStringLiteral("dark")) {
            return Mode::Dark;
        }
        return Mode::Auto;
    }

    [[nodiscard]] static QString modeToSetting(Mode mode) {
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

    [[nodiscard]] static Qt::ColorScheme colorSchemeForMode(Mode mode) {
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
};

} // namespace ui::common
