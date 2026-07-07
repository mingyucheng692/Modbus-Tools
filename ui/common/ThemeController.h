/**
 * @file ThemeController.h
 * @brief Coordinates theme mode state, persistence, system theme tracking, and UI theming.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "ISettingsService.h"
#include "Theme.h"
#include <QObject>
#include <QPalette>

class QIcon;
class QListWidget;
class QMenuBar;
class QStyle;
class QToolButton;
class QWidget;

namespace ui::common {

/**
 * @brief Coordinates theme mode state, persistence, and system theme tracking.
 *
 * Also provides static UI helper functions for applying theme palettes to
 * navigation widgets and the theme toggle button.
 */
class ThemeController : public QObject {
    Q_OBJECT

public:
    explicit ThemeController(ISettingsService& settingsService, QObject* parent = nullptr);

    [[nodiscard]] Theme::Mode currentMode() const noexcept;

    // --- Static UI helpers (formerly ThemeUiHelpers / ThemeVisuals) ---

    static void applyNavigationTheme(const QPalette& palette,
                                     QWidget* navigationPane,
                                     QToolButton* navigationToggleButton,
                                     QListWidget* navigationList);

    static void applyThemeToggleAppearance(const QPalette& palette,
                                           QMenuBar* menuBar,
                                           QStyle* style,
                                           QToolButton* themeToggleButton,
                                           Theme::Mode mode);

public slots:
    void cycleMode();
    void setMode(Theme::Mode mode);

signals:
    void themeChanged();

private:
    void applyMode(Theme::Mode mode, bool persist);
    static QIcon buildModeIcon(const QPalette& palette, Theme::Mode mode, int baseSize);

    ISettingsService& settingsService_;
    Theme::Mode currentMode_ = Theme::Mode::Auto;
};

} // namespace ui::common
