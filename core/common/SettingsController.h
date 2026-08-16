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
#include "ISettingsService.h"

namespace core::common {

/**
 * @brief High-level controller for application-wide settings.
 *        Separates business logic from the raw settings storage service.
 *
 * @par Scope (P2-29)
 *      SettingsController intentionally covers only five cross-cutting
 *      categories that need value-add beyond raw QSettings I/O:
 *      - **Modbus** — qBound clamping against config::Modbus limits
 *        (timeout / retries / retry interval / retry-enabled).
 *      - **Update** — frequency normalization ("off" -> kUpdateCheckNever)
 *        and default fallback to kUpdateCheckStartup when empty.
 *      - **Window state** — main-window geometry / state / navigation collapse.
 *      - **App status** — language and disclaimer-accepted flag.
 *      - **Persistence** — sync() passthrough.
 *
 *      Widget-specific or feature-local keys (e.g. connection-dialog MRU,
 *      analyzer column widths, polling intervals) do NOT belong here; such
 *      callers should use ISettingsService directly via settingsService().
 *      Adding unrelated keys would dilute the controller's purpose and turn
 *      it into a thin QSettings wrapper with no invariant to enforce.
 *
 * @par Why not fold clamping into ISettingsService
 *      ISettingsService is a generic key-value facade shared by every layer;
 *      baking Modbus-specific limits into it would couple the storage
 *      interface to business rules. SettingsController is the appropriate
 *      home for these typed, validated accessors.
 *
 * @par QObject removed (Task 3.2 / P2-1)
 *      This controller uses no signals, slots, or event-loop services — it
 *      is a plain C++ class owned via std::unique_ptr (MainWindow). It must
 *      never be deleteLater()'d; destruction order is the holder's member
 *      declaration order (MainWindow: destroyed with the window, before
 *      the coordinators that received raw pointers into it).
 */
class SettingsController {

public:
    explicit SettingsController(ISettingsService* settingsService);

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

    ISettingsService* settingsService() const { return settingsService_; }

private:
    ISettingsService* settingsService_ = nullptr;
};

} // namespace core::common
