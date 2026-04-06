#pragma once

#include "Theme.h"
#include <QPalette>

class QListWidget;
class QMenuBar;
class QStyle;
class QToolButton;
class QWidget;

namespace ui::common {

class ThemeUiHelpers {
public:
    static void applyNavigationTheme(const QPalette& palette,
                                     QWidget* navigationPane,
                                     QToolButton* navigationToggleButton,
                                     QListWidget* navigationList);

    static void applyThemeToggleAppearance(const QPalette& palette,
                                           QMenuBar* menuBar,
                                           QStyle* style,
                                           QToolButton* themeToggleButton,
                                           Theme::Mode mode);
};

} // namespace ui::common
