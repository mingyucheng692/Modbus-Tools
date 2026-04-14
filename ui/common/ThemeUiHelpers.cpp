/**
 * @file ThemeUiHelpers.cpp
 * @brief Implementation of ThemeUiHelpers.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ThemeUiHelpers.h"

#include "ThemeVisuals.h"

#include <QFrame>
#include <QListWidget>
#include <QMenuBar>
#include <QStyle>
#include <QToolButton>
#include <QWidget>

namespace ui::common {

void ThemeUiHelpers::applyNavigationTheme(const QPalette& palette,
                                          QWidget* navigationPane,
                                          QToolButton* navigationToggleButton,
                                          QListWidget* navigationList) {
    if (!navigationPane && !navigationToggleButton && !navigationList) {
        return;
    }

    if (navigationPane) {
        navigationPane->setAutoFillBackground(true);
        navigationPane->setPalette(palette);
    }
    if (navigationToggleButton) {
        QPalette togglePalette = palette;
        togglePalette.setColor(QPalette::Button, palette.color(QPalette::Window));
        togglePalette.setColor(QPalette::ButtonText, palette.color(QPalette::WindowText));
        navigationToggleButton->setAutoRaise(true);
        navigationToggleButton->setPalette(togglePalette);
    }
    if (navigationList) {
        QPalette listPalette = palette;
        QColor softHighlight = palette.color(QPalette::Highlight);
        softHighlight.setAlpha(qBound(48, softHighlight.alpha(), 96));
        listPalette.setColor(QPalette::Base, palette.color(QPalette::Window));
        listPalette.setColor(QPalette::AlternateBase, palette.color(QPalette::Window));
        listPalette.setColor(QPalette::Text, palette.color(QPalette::WindowText));
        listPalette.setColor(QPalette::Highlight, softHighlight);
        listPalette.setColor(QPalette::HighlightedText, palette.color(QPalette::WindowText));

        navigationList->setStyleSheet(QString());
        navigationList->setAutoFillBackground(true);
        navigationList->setFrameShape(QFrame::NoFrame);
        navigationList->setPalette(listPalette);
        if (navigationList->viewport()) {
            navigationList->viewport()->setAutoFillBackground(true);
            navigationList->viewport()->setPalette(listPalette);
        }
    }
}

void ThemeUiHelpers::applyThemeToggleAppearance(const QPalette& palette,
                                                QMenuBar* menuBar,
                                                QStyle* style,
                                                QToolButton* themeToggleButton,
                                                Theme::Mode mode) {
    if (!menuBar || !style || !themeToggleButton) {
        return;
    }

    const int textHeight = menuBar->fontMetrics().height();
    const int iconExtent = qMax(textHeight, style->pixelMetric(QStyle::PM_SmallIconSize));
    const int buttonExtent = qMax(iconExtent + 6, style->pixelMetric(QStyle::PM_SmallIconSize) + 2);

    themeToggleButton->setFixedSize(buttonExtent, buttonExtent);
    themeToggleButton->setIconSize(QSize(iconExtent, iconExtent));
    themeToggleButton->setIcon(ThemeVisuals::buildModeIcon(palette, mode, iconExtent));
}

} // namespace ui::common
