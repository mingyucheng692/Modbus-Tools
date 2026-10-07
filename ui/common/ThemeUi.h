// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "Theme.h"
#include <QPalette>

class QListWidget;
class QMenuBar;
class QStyle;
class QToolButton;
class QWidget;

namespace ui::common::theme_ui {

void applyNavigationTheme(const QPalette& palette,
                          QWidget* navigationPane,
                          QToolButton* navigationToggleButton,
                          QListWidget* navigationList);

void applyThemeToggleAppearance(const QPalette& palette,
                                QMenuBar* menuBar,
                                QStyle* style,
                                QToolButton* themeToggleButton,
                                Theme::Mode mode);

} // namespace ui::common::theme_ui
