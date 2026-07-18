/**
 * @file ThemeController.h
 * @brief Coordinates theme mode state, persistence, and system theme tracking.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../../core/common/ISettingsService.h"
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
 */
class ThemeController : public QObject {
    Q_OBJECT

public:
    explicit ThemeController(::core::common::ISettingsService& settingsService, QObject* parent = nullptr);

    [[nodiscard]] Theme::Mode currentMode() const noexcept;

public slots:
    void cycleMode();
    void setMode(Theme::Mode mode);

signals:
    void themeChanged();

private:
    void applyMode(Theme::Mode mode, bool persist);

    ::core::common::ISettingsService& settingsService_;
    Theme::Mode currentMode_ = Theme::Mode::Auto;
};

} // namespace ui::common
