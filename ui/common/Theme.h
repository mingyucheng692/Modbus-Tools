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

namespace ui::common {

class Theme {
public:
    enum class Mode {
        Auto,
        Light,
        Dark
    };

    static Mode modeFromSetting(const QString& value);
    static QString modeToSetting(Mode mode);
    static Qt::ColorScheme colorSchemeForMode(Mode mode);
};

} // namespace ui::common
