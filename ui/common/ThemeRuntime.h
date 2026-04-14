/**
 * @file ThemeRuntime.h
 * @brief Header file for ThemeRuntime.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "Theme.h"
#include <QObject>

namespace ui::common {

/**
 * @brief Abstracts platform-specific theme application and system theme events.
 */
class ThemeRuntime : public QObject {
    Q_OBJECT

public:
    explicit ThemeRuntime(QObject* parent = nullptr) : QObject(parent) {}
    ~ThemeRuntime() override = default;

    virtual void applyMode(Theme::Mode mode) = 0;

signals:
    void systemThemeChanged();
};

} // namespace ui::common
