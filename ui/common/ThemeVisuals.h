/**
 * @file ThemeVisuals.h
 * @brief Header file for ThemeVisuals.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "Theme.h"
#include <QIcon>
#include <QPalette>

namespace ui::common {

/**
 * @brief Provides theme-driven visual helpers for icons.
 */
class ThemeVisuals {
public:
    [[nodiscard]] static QIcon buildModeIcon(const QPalette& palette, Theme::Mode mode, int baseSize);
};

} // namespace ui::common
