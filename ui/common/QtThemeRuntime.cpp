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
