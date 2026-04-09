#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QByteArray>
#include "../../ui/common/ISettingsService.h"

namespace core::common {

/**
 * @brief High-level controller for application-wide settings.
 *        Separates business logic from the raw settings storage service.
 */
class SettingsController : public QObject {
    Q_OBJECT

public:
    explicit SettingsController(ui::common::ISettingsService* settingsService, QObject* parent = nullptr);

    // Modbus Settings
    void setModbusSettings(int timeoutMs, int retries, int retryIntervalMs, bool retryEnabled);
    void loadModbusSettings(int& timeoutMs, int& retries, int& retryIntervalMs, bool& retryEnabled);

    // Update Settings
    QString updateCheckFrequency() const;
    void setUpdateCheckFrequency(const QString& frequency);
    
    QString lastUpdateCheckUtc() const;
    void setLastUpdateCheckUtc(const QString& utcTime);

    // Window State
    QByteArray mainWindowGeometry() const;
    void setMainWindowGeometry(const QByteArray& geometry);
    
    QByteArray mainWindowState() const;
    void setMainWindowState(const QByteArray& state);

    bool navigationCollapsed() const;
    void setNavigationCollapsed(bool collapsed);

    // App Status
    QString language() const;
    void setLanguage(const QString& locale);

    bool disclaimerAccepted() const;
    void setDisclaimerAccepted(bool accepted);

    // Persistence
    void sync();

    ui::common::ISettingsService* settingsService() const { return settingsService_; }

private:
    ui::common::ISettingsService* settingsService_ = nullptr;
};

} // namespace core::common
