#pragma once

#include "ISettingsService.h"
#include "Theme.h"
#include "ThemeRuntime.h"
#include <QObject>

namespace ui::common {

/**
 * @brief Coordinates theme mode state, persistence, and system theme tracking.
 */
class ThemeController : public QObject {
    Q_OBJECT

public:
    explicit ThemeController(ThemeRuntime& themeRuntime,
                             ISettingsService& settingsService,
                             QObject* parent = nullptr);

    [[nodiscard]] Theme::Mode currentMode() const noexcept;

public slots:
    void cycleMode();
    void setMode(Theme::Mode mode);

signals:
    void themeChanged();

private:
    void applyMode(Theme::Mode mode, bool persist);

    ThemeRuntime& themeRuntime_;
    ISettingsService& settingsService_;
    Theme::Mode currentMode_ = Theme::Mode::Auto;
};

} // namespace ui::common
