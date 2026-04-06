#pragma once

#include <QApplication>

namespace ui::common {

class Theme {
public:
    enum class Mode {
        Auto,
        Light,
        Dark
    };

    static void apply(QApplication& app, Mode mode);
    static void applyAuto(QApplication& app);
    static void applyLight(QApplication& app);
    static void applyDark(QApplication& app);

    static Mode modeFromSetting(const QString& value);
    static QString modeToSetting(Mode mode);
};

} // namespace ui::common
