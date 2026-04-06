#pragma once

#include "ThemeRuntime.h"

class QApplication;

namespace ui::common {

/**
 * @brief Qt-based theme runtime using QStyleHints color scheme APIs.
 */
class QtThemeRuntime final : public ThemeRuntime {
    Q_OBJECT

public:
    explicit QtThemeRuntime(QApplication& application, QObject* parent = nullptr);

    void applyMode(Theme::Mode mode) override;

private:
    QApplication& application_;
};

} // namespace ui::common
