#pragma once

#include <QByteArray>
#include <QString>

namespace ui::application {

class IMainWindowView {
public:
    virtual ~IMainWindowView() = default;

    virtual void initializeUi() = 0;
    virtual void setNavigationCollapsed(bool collapsed) = 0;
    [[nodiscard]] virtual bool isNavigationCollapsed() const = 0;
    virtual void restoreWindowGeometry(const QByteArray& geometry) = 0;
    virtual void restoreWindowState(const QByteArray& state) = 0;
    [[nodiscard]] virtual QByteArray saveWindowGeometry() const = 0;
    [[nodiscard]] virtual QByteArray saveWindowState() const = 0;
    virtual void applyModbusSettings(int timeoutMs, int retries, int retryIntervalMs) = 0;
    virtual void openModbusSettingsDialog() = 0;
    virtual void openUpdateSettingsDialog() = 0;
    virtual void openAboutDialog() = 0;
    [[nodiscard]] virtual bool showDisclaimerDialog() = 0;
    virtual void retranslateUi(const QString& effectiveLocale) = 0;
};

} // namespace ui::application
