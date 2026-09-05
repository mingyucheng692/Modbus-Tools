/**
 * @file SettingsController.h
 * @brief Header file for SettingsController.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <QVariant>
#include <QByteArray>
#include <optional>

#include "modbus/base/ModbusAddressMapping.h"

namespace infra::config {
class ISettingsService;
}

namespace core::common {

/**
 * @brief High-level controller for application-wide settings.
 *        Separates business logic from the raw settings storage service.
 */
class SettingsController {

public:
    explicit SettingsController(infra::config::ISettingsService* settingsService);

    // Modbus Settings
    void setModbusSettings(int timeoutMs, int retries, int retryIntervalMs, bool retryEnabled);
    void loadModbusSettings(int& timeoutMs, int& retries, int& retryIntervalMs, bool& retryEnabled);

    // Modbus Address Base
    [[nodiscard]] modbus::address::AddressBase addressBase() const;
    void setAddressBase(modbus::address::AddressBase base);

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

    // Logging Settings
    [[nodiscard]] std::optional<int> logLevel() const;
    void setLogLevel(int level);

    // Persistence
    void sync();

    infra::config::ISettingsService* settingsService() const { return settingsService_; }

private:
    infra::config::ISettingsService* settingsService_ = nullptr;
};

} // namespace core::common
