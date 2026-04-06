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
