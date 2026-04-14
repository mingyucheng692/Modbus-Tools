/**
 * @file SettingsController.cpp
 * @brief Implementation of SettingsController.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "SettingsController.h"
#include "../../ui/common/SettingsKeys.h"
#include "../AppConstants.h"

namespace core::common {

using namespace ui::common::settings_keys;

SettingsController::SettingsController(ui::common::ISettingsService* settingsService, QObject* parent)
    : QObject(parent), settingsService_(settingsService) {
    Q_ASSERT(settingsService_ != nullptr);
}

void SettingsController::setModbusSettings(int timeoutMs, int retries, int retryIntervalMs, bool retryEnabled) {
    settingsService_->setValue(kModbusTimeoutMs, timeoutMs);
    settingsService_->setValue(kModbusRetryCount, retries);
    settingsService_->setValue(kModbusRetryIntervalMs, retryIntervalMs);
    settingsService_->setValue(kModbusRetryEnabled, retryEnabled);
}

void SettingsController::loadModbusSettings(int& timeoutMs, int& retries, int& retryIntervalMs, bool& retryEnabled) {
    using namespace app::constants;
    timeoutMs = qBound(Values::Modbus::kMinTimeoutMs,
                       settingsService_->value(kModbusTimeoutMs).toInt(),
                       Values::Modbus::kMaxTimeoutMs);
    retries = qBound(Values::Modbus::kMinRetryCount,
                     settingsService_->value(kModbusRetryCount).toInt(),
                     Values::Modbus::kMaxRetryCount);
    retryIntervalMs = qBound(Values::Modbus::kMinRetryIntervalMs,
                             settingsService_->value(kModbusRetryIntervalMs).toInt(),
                             Values::Modbus::kMaxRetryIntervalMs);
    retryEnabled = settingsService_->value(kModbusRetryEnabled).toBool();
}

QString SettingsController::updateCheckFrequency() const {
    QString freq = settingsService_->value(kAppUpdateCheckFrequency).toString();
    if (freq.isEmpty()) {
        return QLatin1String(app::constants::Values::App::kUpdateCheckStartup);
    }
    return freq;
}

void SettingsController::setUpdateCheckFrequency(const QString& frequency) {
    settingsService_->setValue(kAppUpdateCheckFrequency, frequency);
}

QString SettingsController::lastUpdateCheckUtc() const {
    return settingsService_->value(kAppUpdateLastCheckUtc).toString();
}

void SettingsController::setLastUpdateCheckUtc(const QString& utcTime) {
    settingsService_->setValue(kAppUpdateLastCheckUtc, utcTime);
}

QByteArray SettingsController::mainWindowGeometry() const {
    return settingsService_->value(kAppMainWindowGeometry).toByteArray();
}

void SettingsController::setMainWindowGeometry(const QByteArray& geometry) {
    settingsService_->setValue(kAppMainWindowGeometry, geometry);
}

QByteArray SettingsController::mainWindowState() const {
    return settingsService_->value(kAppMainWindowState).toByteArray();
}

void SettingsController::setMainWindowState(const QByteArray& state) {
    settingsService_->setValue(kAppMainWindowState, state);
}

bool SettingsController::navigationCollapsed() const {
    return settingsService_->value(kAppNavigationCollapsed).toBool();
}

void SettingsController::setNavigationCollapsed(bool collapsed) {
    settingsService_->setValue(kAppNavigationCollapsed, collapsed);
}

QString SettingsController::language() const {
    return settingsService_->value(kAppLanguage).toString();
}

void SettingsController::setLanguage(const QString& locale) {
    settingsService_->setValue(kAppLanguage, locale);
}

bool SettingsController::disclaimerAccepted() const {
    return settingsService_->value(kAppDisclaimerAccepted).toBool();
}

void SettingsController::setDisclaimerAccepted(bool accepted) {
    settingsService_->setValue(kAppDisclaimerAccepted, accepted);
}

void SettingsController::sync() {
    settingsService_->sync();
}

} // namespace core::common
