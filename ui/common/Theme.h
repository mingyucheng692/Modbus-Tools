#pragma once

#include <QtCore/Qt>

namespace ui::common {

class Theme {
public:
    enum class Mode {
        Auto,
        Light,
        Dark
    };

    static Mode modeFromSetting(const QString& value);
    static QString modeToSetting(Mode mode);
    static Qt::ColorScheme colorSchemeForMode(Mode mode);
};

} // namespace ui::common
