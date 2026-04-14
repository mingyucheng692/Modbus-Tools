/**
 * @file QtThemeRuntime.cpp
 * @brief Implementation of QtThemeRuntime.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "QtThemeRuntime.h"
#include <QApplication>
#include <QStyleHints>

namespace ui::common {

QtThemeRuntime::QtThemeRuntime(QApplication& application, QObject* parent)
    : ThemeRuntime(parent),
      application_(application) {
    connect(application_.styleHints(),
            &QStyleHints::colorSchemeChanged,
            this,
            [this](Qt::ColorScheme) {
                emit systemThemeChanged();
            });
}

void QtThemeRuntime::applyMode(Theme::Mode mode) {
    application_.styleHints()->setColorScheme(Theme::colorSchemeForMode(mode));
}

} // namespace ui::common
