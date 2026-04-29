#pragma once

#include <QString>

namespace ui::application {

class IMainWindowView {
public:
    virtual ~IMainWindowView() = default;

    virtual void initializeUi() = 0;
    virtual void setNavigationCollapsed(bool collapsed) = 0;
    [[nodiscard]] virtual bool isNavigationCollapsed() const = 0;
    virtual void openModbusSettingsDialog() = 0;
    virtual void openUpdateSettingsDialog() = 0;
    virtual void openAboutDialog() = 0;
    virtual void retranslateUi(const QString& effectiveLocale) = 0;
    virtual void persistWindowState() = 0;
};

} // namespace ui::application
