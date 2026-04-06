#include "Theme.h"
#include <QStyleHints>

namespace ui::common {

void Theme::apply(QApplication& app, Mode mode) {
    switch (mode) {
    case Mode::Auto:
        applyAuto(app);
        break;
    case Mode::Light:
        applyLight(app);
        break;
    case Mode::Dark:
        applyDark(app);
        break;
    }
}

void Theme::applyAuto(QApplication& app) {
    app.styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
}

void Theme::applyLight(QApplication& app) {
    app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
}

void Theme::applyDark(QApplication& app) {
    app.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
}

Theme::Mode Theme::modeFromSetting(const QString& value) {
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("light")) {
        return Mode::Light;
    }
    if (normalized == QStringLiteral("dark")) {
        return Mode::Dark;
    }
    return Mode::Auto;
}

QString Theme::modeToSetting(Mode mode) {
    switch (mode) {
    case Mode::Auto:
        return QStringLiteral("auto");
    case Mode::Light:
        return QStringLiteral("light");
    case Mode::Dark:
        return QStringLiteral("dark");
    }
    return QStringLiteral("auto");
}

} // namespace ui::common
