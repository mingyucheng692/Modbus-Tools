/**
 * @file SettingsService.cpp
 * @brief Implementation of SettingsService.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "SettingsService.h"
#include "AppConfig.h"
#include "SettingsKeys.h"
#include "AppConstants.h"
#include <QCoreApplication>
#include <QTimer>

namespace ui::common {

SettingsService::SettingsService(QObject* parent, std::unique_ptr<IAppConfig> appConfig)
    : QObject(parent),
      appConfig_(appConfig ? std::move(appConfig) : std::make_unique<AppConfig>()),
      syncTimer_(new QTimer(this)) {
    syncTimer_->setSingleShot(true);
    syncTimer_->setInterval(app::constants::Values::Settings::kSyncDebounceMs);
    connect(syncTimer_, &QTimer::timeout, this, &SettingsService::sync);

    initializeDefaults();
    load();

    if (auto* app = QCoreApplication::instance()) {
        connect(app, &QCoreApplication::aboutToQuit, this, &SettingsService::sync);
    }
}

QVariant SettingsService::value(const QString& key) const {
    return values_.value(key, defaults_.value(key));
}

bool SettingsService::contains(const QString& key) const {
    return loadedKeys_.contains(key) || dirtyKeys_.contains(key);
}

void SettingsService::setValue(const QString& key, const QVariant& value) {
    const QVariant currentValue = values_.contains(key) ? values_.value(key) : defaults_.value(key);
    const bool alreadyTracked = loadedKeys_.contains(key) || dirtyKeys_.contains(key);
    if (currentValue == value && alreadyTracked) {
        return;
    }
    values_.insert(key, value);
    loadedKeys_.insert(key);
    keysToRemove_.remove(key);
    dirtyKeys_.insert(key);
    scheduleSync();
}

QString SettingsService::configFilePath() const {
    return appConfig_->configFilePath();
}

void SettingsService::sync() {
    if (dirtyKeys_.isEmpty() && keysToRemove_.isEmpty()) {
        return;
    }

    for (auto it = dirtyKeys_.cbegin(); it != dirtyKeys_.cend(); ++it) {
        appConfig_->setValue(*it, values_.value(*it, defaults_.value(*it)));
        loadedKeys_.insert(*it);
    }
    for (auto it = keysToRemove_.cbegin(); it != keysToRemove_.cend(); ++it) {
        appConfig_->remove(*it);
        loadedKeys_.remove(*it);
    }
    appConfig_->sync();
    dirtyKeys_.clear();
    keysToRemove_.clear();
}

void SettingsService::initializeDefaults() {
    using namespace settings_keys;

    defaults_.insert(kAppLanguage, QStringLiteral("system"));
    defaults_.insert(kAppThemeMode, QStringLiteral("auto"));
    defaults_.insert(kAppNavigationCollapsed, false);
    defaults_.insert(kAppUpdateCheckFrequency, QStringLiteral("startup"));
    defaults_.insert(kAppUpdateLastCheckUtc, QString());
    defaults_.insert(kAppDisclaimerAccepted, false);
    defaults_.insert(kAppMainWindowGeometry, QByteArray());
    defaults_.insert(kAppMainWindowState, QByteArray());

    defaults_.insert(kModbusTimeoutMs, app::constants::Values::Modbus::kDefaultTimeoutMs);
    defaults_.insert(kModbusRetryCount, app::constants::Values::Modbus::kDefaultRetryCount);
    defaults_.insert(kModbusRetryIntervalMs, app::constants::Values::Modbus::kDefaultRetryIntervalMs);
    defaults_.insert(kModbusRetryEnabled, app::constants::Values::Modbus::kDefaultRetryEnabled);

    defaults_.insert(kFrameAnalyzerStartAddr, app::constants::Values::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kFrameAnalyzerDecodeMode, app::constants::Values::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kFrameAnalyzerHistoryCollapsed, false);

    defaults_.insert(kModbusTcpIp, QString::fromLatin1(app::constants::Values::Network::kDefaultDeviceAddress));
    defaults_.insert(kModbusTcpPort, app::constants::Values::Network::kDefaultModbusTcpPort);
    defaults_.insert(kModbusTcpConnectionCollapsed, false);
    defaults_.insert(kModbusTcpStandardSlaveId, app::constants::Values::Modbus::kDefaultSlaveId);
    defaults_.insert(kModbusTcpStandardStartAddr, app::constants::Values::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kModbusTcpStandardQuantity, app::constants::Values::Modbus::kDefaultStandardQuantity);
    defaults_.insert(kModbusTcpStandardFormatIndex, app::constants::Values::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kModbusTcpStandardCollapsed, false);
    defaults_.insert(kModbusTcpRawCollapsed, true);
    defaults_.insert(kModbusTcpTrafficAutoScroll, true);
    defaults_.insert(kModbusTcpTrafficShowTx, true);
    defaults_.insert(kModbusTcpTrafficShowRx, true);
    defaults_.insert(kModbusTcpTrafficCollapsed, false);
    defaults_.insert(kModbusTcpControlEnablePoll, false);
    defaults_.insert(kModbusTcpControlIntervalMs, app::constants::Values::Modbus::kDefaultControlIntervalMs);
    defaults_.insert(kModbusTcpControlFcIndex, app::constants::Values::Modbus::kDefaultControlFunctionIndex);
    defaults_.insert(kModbusTcpControlAddr, app::constants::Values::Modbus::kDefaultControlAddress);
    defaults_.insert(kModbusTcpControlQty, app::constants::Values::Modbus::kDefaultControlQuantity);
    defaults_.insert(kModbusTcpDataMonitorCollapsed, false);

    defaults_.insert(kModbusRtuBaudRate, QString::fromLatin1(app::constants::Values::Serial::kDefaultBaudRateText));
    defaults_.insert(kModbusRtuDataBits, QString::fromLatin1(app::constants::Values::Serial::kDefaultDataBitsText));
    defaults_.insert(kModbusRtuParity, QString::fromLatin1(app::constants::Values::Serial::kDefaultParityText));
    defaults_.insert(kModbusRtuStopBits, QString::fromLatin1(app::constants::Values::Serial::kDefaultStopBitsText));
    defaults_.insert(kModbusRtuPortName, QString());
    defaults_.insert(kModbusRtuConnectionCollapsed, false);
    defaults_.insert(kModbusRtuStandardSlaveId, app::constants::Values::Modbus::kDefaultSlaveId);
    defaults_.insert(kModbusRtuStandardStartAddr, app::constants::Values::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kModbusRtuStandardQuantity, app::constants::Values::Modbus::kDefaultStandardQuantity);
    defaults_.insert(kModbusRtuStandardFormatIndex, app::constants::Values::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kModbusRtuStandardCollapsed, false);
    defaults_.insert(kModbusRtuRawCollapsed, true);
    defaults_.insert(kModbusRtuTrafficAutoScroll, true);
    defaults_.insert(kModbusRtuTrafficShowTx, true);
    defaults_.insert(kModbusRtuTrafficShowRx, true);
    defaults_.insert(kModbusRtuTrafficCollapsed, false);
    defaults_.insert(kModbusRtuControlEnablePoll, false);
    defaults_.insert(kModbusRtuControlIntervalMs, app::constants::Values::Modbus::kDefaultControlIntervalMs);
    defaults_.insert(kModbusRtuControlFcIndex, app::constants::Values::Modbus::kDefaultControlFunctionIndex);
    defaults_.insert(kModbusRtuControlAddr, app::constants::Values::Modbus::kDefaultControlAddress);
    defaults_.insert(kModbusRtuControlQty, app::constants::Values::Modbus::kDefaultControlQuantity);
    defaults_.insert(kModbusRtuDataMonitorCollapsed, false);

    // New string-based keys for smart hex/dec input support
    defaults_.insert(QString(kModbusTcpStandardSlaveId) + "Str", QString::number(app::constants::Values::Modbus::kDefaultSlaveId));
    defaults_.insert(QString(kModbusTcpStandardStartAddr) + "Str", QString::number(app::constants::Values::Modbus::kDefaultStandardStartAddress));
    defaults_.insert(QString(kModbusRtuStandardSlaveId) + "Str", QString::number(app::constants::Values::Modbus::kDefaultSlaveId));
    defaults_.insert(QString(kModbusRtuStandardStartAddr) + "Str", QString::number(app::constants::Values::Modbus::kDefaultStandardStartAddress));

    defaults_.insert(kTcpClientIp, QString::fromLatin1(app::constants::Values::Network::kDefaultDeviceAddress));
    defaults_.insert(kTcpClientPort, app::constants::Values::Network::kDefaultGenericTcpPort);
    defaults_.insert(kTcpClientConnectionCollapsed, false);
    defaults_.insert(kTcpClientTrafficAutoScroll, true);
    defaults_.insert(kTcpClientTrafficShowTx, true);
    defaults_.insert(kTcpClientTrafficShowRx, true);
    defaults_.insert(kTcpClientTrafficCollapsed, false);
    defaults_.insert(kTcpClientInputFormat, QStringLiteral("hex"));
    defaults_.insert(kTcpClientInputAutoSend, false);
    defaults_.insert(kTcpClientInputIntervalMs, app::constants::Values::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kTcpClientInputCollapsed, false);

    defaults_.insert(kSerialPortBaudRate, QString::fromLatin1(app::constants::Values::Serial::kDefaultBaudRateText));
    defaults_.insert(kSerialPortDataBits, QString::fromLatin1(app::constants::Values::Serial::kDefaultDataBitsText));
    defaults_.insert(kSerialPortParity, QString::fromLatin1(app::constants::Values::Serial::kDefaultParityText));
    defaults_.insert(kSerialPortStopBits, QString::fromLatin1(app::constants::Values::Serial::kDefaultStopBitsText));
    defaults_.insert(kSerialPortPortName, QString());
    defaults_.insert(kSerialPortConnectionCollapsed, false);
    defaults_.insert(kSerialPortTrafficAutoScroll, true);
    defaults_.insert(kSerialPortTrafficShowTx, true);
    defaults_.insert(kSerialPortTrafficShowRx, true);
    defaults_.insert(kSerialPortTrafficCollapsed, false);
    defaults_.insert(kSerialPortInputFormat, QStringLiteral("hex"));
    defaults_.insert(kSerialPortInputAutoSend, false);
    defaults_.insert(kSerialPortInputIntervalMs, app::constants::Values::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kSerialPortInputCollapsed, false);
    defaults_.insert(kSerialPortDtr, false);
    defaults_.insert(kSerialPortRts, false);
}

void SettingsService::load() {
    // Load explicitly defined defaults first (ensures baseline exists)
    for (auto it = defaults_.cbegin(); it != defaults_.cend(); ++it) {
        values_.insert(it.key(), appConfig_->value(it.key(), it.value()));
    }

    // Now load everything else found in the file to handle dynamic/prefixed keys
    const QStringList allKeys = appConfig_->allKeys();
    for (const QString& key : allKeys) {
        if (!values_.contains(key)) {
            values_.insert(key, appConfig_->value(key));
        }
        loadedKeys_.insert(key);
    }

    using namespace settings_keys;
    if (appConfig_->contains(kLegacySerialBaudRate) && !appConfig_->contains(kModbusRtuBaudRate)) {
        values_.insert(kModbusRtuBaudRate, appConfig_->value(kLegacySerialBaudRate));
        dirtyKeys_.insert(kModbusRtuBaudRate);
        keysToRemove_.insert(kLegacySerialBaudRate);
    }
    scheduleSync();
}

void SettingsService::scheduleSync() {
    syncTimer_->start();
}

} // namespace ui::common
