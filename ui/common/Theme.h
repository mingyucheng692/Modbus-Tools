#pragma once

#include <QApplication>

namespace ui::common {

class Theme {
public:
    static void applyLight(QApplication& app);
    static void applyDark(QApplication& app);
    
    // Helper to get navigation style based on current theme
    static QString getNavigationStyle(bool isDark);
};

} // namespace ui::common
