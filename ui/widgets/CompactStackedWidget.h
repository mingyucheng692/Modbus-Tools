// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QStackedWidget>
#include <QSize>

namespace ui::widgets {

class CompactStackedWidget : public QStackedWidget {
public:
    using QStackedWidget::QStackedWidget;

    QSize sizeHint() const override {
        if (auto* curr = currentWidget()) {
            return curr->sizeHint();
        }
        return QStackedWidget::sizeHint();
    }

    QSize minimumSizeHint() const override {
        if (auto* curr = currentWidget()) {
            return curr->minimumSizeHint();
        }
        return QStackedWidget::minimumSizeHint();
    }
};

} // namespace ui::widgets
