#pragma once

#include "Theme.h"
#include <QIcon>
#include <QPalette>
#include <QString>

namespace ui::common {

/**
 * @brief Provides theme-driven visual helpers for icons and widget styling.
 */
class ThemeVisuals {
public:
    [[nodiscard]] static QIcon buildModeIcon(const QPalette& palette, Theme::Mode mode, int baseSize);
    [[nodiscard]] static QString navigationListStyle(const QPalette& palette);
};

} // namespace ui::common
