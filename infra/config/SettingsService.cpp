/**
 * @file SettingsService.cpp
 * @brief Implementation of SettingsService.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "SettingsService.h"
#include "core/common/SettingsKeys.h"
#include "core/Config.h"
#include "infra/platform/PathResolver.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QTimer>

namespace {

QString resolveConfigFilePath(const infra::platform::PathResolver& pathResolver)
{
    return QDir(pathResolver.resolveConfigDir()).filePath(QStringLiteral("config.ini"));
}

} // namespace

namespace infra::config {

SettingsService::SettingsService(infra::platform::PathResolver& pathResolver,
                                 QObject* parent)
    : QObject(parent),
      settings_(std::make_unique<QSettings>(resolveConfigFilePath(pathResolver), QSettings::IniFormat)),
      syncTimer_(new QTimer(this)),
      pathResolver_(pathResolver) {
    syncTimer_->setSingleShot(true);
    syncTimer_->setInterval(::config::Settings::kSyncDebounceMs);
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
    return values_.contains(key) || dirtyKeys_.contains(key);
}

void SettingsService::setValue(const QString& key, const QVariant& value) {
    const QVariant currentValue = values_.contains(key) ? values_.value(key) : defaults_.value(key);
    const bool alreadyTracked = values_.contains(key) || dirtyKeys_.contains(key);
    if (currentValue == value && alreadyTracked) {
        return;
    }
    values_.insert(key, value);
    keysToRemove_.remove(key);
    dirtyKeys_.insert(key);
    scheduleSync();
}

QString SettingsService::configFilePath() const {
    return settings_->fileName();
}

void SettingsService::sync() {
    if (dirtyKeys_.isEmpty() && keysToRemove_.isEmpty()) {
        return;
    }

    for (auto it = dirtyKeys_.cbegin(); it != dirtyKeys_.cend(); ++it) {
        settings_->setValue(*it, values_.value(*it, defaults_.value(*it)));
    }
    for (auto it = keysToRemove_.cbegin(); it != keysToRemove_.cend(); ++it) {
        settings_->remove(*it);
        values_.remove(*it);
    }
    settings_->sync();
    dirtyKeys_.clear();
    keysToRemove_.clear();
}

void SettingsService::initializeDefaults() {
    using namespace core::common::settings_keys;

    defaults_.insert(kAppLanguage, QStringLiteral("system"));
    defaults_.insert(kAppThemeMode, QStringLiteral("auto"));
    defaults_.insert(kAppNavigationCollapsed, false);
    defaults_.insert(kAppUpdateCheckFrequency, QStringLiteral("startup"));
    defaults_.insert(kAppUpdateLastCheckUtc, QString());
    defaults_.insert(kAppDisclaimerAccepted, false);
    defaults_.insert(kAppMainWindowGeometry, QByteArray());
    defaults_.insert(kAppMainWindowState, QByteArray());

    defaults_.insert(kModbusTimeoutMs, ::config::Modbus::kDefaultTimeoutMs);
    defaults_.insert(kModbusRetryCount, ::config::Modbus::kDefaultRetryCount);
    defaults_.insert(kModbusRetryIntervalMs, ::config::Modbus::kDefaultRetryIntervalMs);
    defaults_.insert(kModbusRetryEnabled, ::config::Modbus::kDefaultRetryEnabled);

    defaults_.insert(kFrameAnalyzerStartAddr, ::config::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kFrameAnalyzerDecodeMode, ::config::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kFrameAnalyzerHistoryCollapsed, false);

    defaults_.insert(kModbusTcpIp, QString::fromLatin1(::config::Network::kDefaultDeviceAddress));
    defaults_.insert(kModbusTcpPort, ::config::Network::kDefaultModbusTcpPort);
    defaults_.insert(kModbusTcpConnectionCollapsed, false);
    defaults_.insert(kModbusTcpStandardSlaveId, ::config::Modbus::kDefaultSlaveId);
    defaults_.insert(kModbusTcpStandardStartAddr, ::config::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kModbusTcpStandardQuantity, ::config::Modbus::kDefaultStandardQuantity);
    defaults_.insert(kModbusTcpStandardFormatIndex, ::config::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kModbusTcpStandardCollapsed, false);
    defaults_.insert(kModbusTcpRawCollapsed, true);
    defaults_.insert(kModbusTcpTrafficAutoScroll, true);
    defaults_.insert(kModbusTcpTrafficShowTx, true);
    defaults_.insert(kModbusTcpTrafficShowRx, true);
    defaults_.insert(kModbusTcpTrafficCollapsed, false);
    defaults_.insert(kModbusTcpControlEnablePoll, false);
    defaults_.insert(kModbusTcpControlIntervalMs, ::config::Modbus::kDefaultControlIntervalMs);
    defaults_.insert(kModbusTcpControlFcIndex, ::config::Modbus::kDefaultControlFunctionIndex);
    defaults_.insert(kModbusTcpControlAddr, ::config::Modbus::kDefaultControlAddress);
    defaults_.insert(kModbusTcpControlQty, ::config::Modbus::kDefaultControlQuantity);
    defaults_.insert(kModbusTcpDataMonitorCollapsed, false);

    defaults_.insert(kModbusRtuBaudRate, QString::fromLatin1(::config::Serial::kDefaultBaudRateText));
    defaults_.insert(kModbusRtuDataBits, QString::fromLatin1(::config::Serial::kDefaultDataBitsText));
    defaults_.insert(kModbusRtuParity, QString::fromLatin1(::config::Serial::kDefaultParityText));
    defaults_.insert(kModbusRtuStopBits, QString::fromLatin1(::config::Serial::kDefaultStopBitsText));
    defaults_.insert(kModbusRtuPortName, QString());
    defaults_.insert(kModbusRtuConnectionCollapsed, false);
    defaults_.insert(kModbusRtuStandardSlaveId, ::config::Modbus::kDefaultSlaveId);
    defaults_.insert(kModbusRtuStandardStartAddr, ::config::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kModbusRtuStandardQuantity, ::config::Modbus::kDefaultStandardQuantity);
    defaults_.insert(kModbusRtuStandardFormatIndex, ::config::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kModbusRtuStandardCollapsed, false);
    defaults_.insert(kModbusRtuRawCollapsed, true);
    defaults_.insert(kModbusRtuTrafficAutoScroll, true);
    defaults_.insert(kModbusRtuTrafficShowTx, true);
    defaults_.insert(kModbusRtuTrafficShowRx, true);
    defaults_.insert(kModbusRtuTrafficCollapsed, false);
    defaults_.insert(kModbusRtuControlEnablePoll, false);
    defaults_.insert(kModbusRtuControlIntervalMs, ::config::Modbus::kDefaultControlIntervalMs);
    defaults_.insert(kModbusRtuControlFcIndex, ::config::Modbus::kDefaultControlFunctionIndex);
    defaults_.insert(kModbusRtuControlAddr, ::config::Modbus::kDefaultControlAddress);
    defaults_.insert(kModbusRtuControlQty, ::config::Modbus::kDefaultControlQuantity);
    defaults_.insert(kModbusRtuDataMonitorCollapsed, false);

    defaults_.insert(QString(kModbusTcpStandardSlaveId) + "Str", QString::number(::config::Modbus::kDefaultSlaveId));
    defaults_.insert(QString(kModbusTcpStandardStartAddr) + "Str", QString::number(::config::Modbus::kDefaultStandardStartAddress));
    defaults_.insert(QString(kModbusRtuStandardSlaveId) + "Str", QString::number(::config::Modbus::kDefaultSlaveId));
    defaults_.insert(QString(kModbusRtuStandardStartAddr) + "Str", QString::number(::config::Modbus::kDefaultStandardStartAddress));

    // Modbus ASCII mode defaults (symmetric with RTU serial transport + standard/traffic/control)
    defaults_.insert(QStringLiteral("modbus/ascii/serial/baudRate"), QString::fromLatin1(::config::Serial::kDefaultBaudRateText));
    defaults_.insert(QStringLiteral("modbus/ascii/serial/dataBits"), QString::fromLatin1(::config::Serial::kDefaultDataBitsText));
    defaults_.insert(QStringLiteral("modbus/ascii/serial/parity"), QString::fromLatin1(::config::Serial::kDefaultParityText));
    defaults_.insert(QStringLiteral("modbus/ascii/serial/stopBits"), QString::fromLatin1(::config::Serial::kDefaultStopBitsText));
    defaults_.insert(QStringLiteral("modbus/ascii/serial/portName"), QString());
    defaults_.insert(QStringLiteral("modbus/ascii/serial/ui/connectionSettingsCollapsed"), false);
    defaults_.insert(QStringLiteral("modbus/ascii/standard/slaveId"), ::config::Modbus::kDefaultSlaveId);
    defaults_.insert(QStringLiteral("modbus/ascii/standard/startAddr"), ::config::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(QStringLiteral("modbus/ascii/standard/quantity"), ::config::Modbus::kDefaultStandardQuantity);
    defaults_.insert(QStringLiteral("modbus/ascii/standard/formatIndex"), ::config::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(QStringLiteral("modbus/ascii/standard/ui/standardCollapsed"), false);
    defaults_.insert(QStringLiteral("modbus/ascii/standard/ui/rawCollapsed"), true);
    defaults_.insert(QStringLiteral("modbus/ascii/traffic/autoScroll"), true);
    defaults_.insert(QStringLiteral("modbus/ascii/traffic/showTx"), true);
    defaults_.insert(QStringLiteral("modbus/ascii/traffic/showRx"), true);
    defaults_.insert(QStringLiteral("modbus/ascii/traffic/ui/trafficMonitorCollapsed"), false);
    defaults_.insert(QStringLiteral("modbus/ascii/control/enablePoll"), false);
    defaults_.insert(QStringLiteral("modbus/ascii/control/intervalMs"), ::config::Modbus::kDefaultControlIntervalMs);
    defaults_.insert(QStringLiteral("modbus/ascii/control/fcIndex"), ::config::Modbus::kDefaultControlFunctionIndex);
    defaults_.insert(QStringLiteral("modbus/ascii/control/addr"), ::config::Modbus::kDefaultControlAddress);
    defaults_.insert(QStringLiteral("modbus/ascii/control/qty"), ::config::Modbus::kDefaultControlQuantity);
    defaults_.insert(QStringLiteral("modbus/ascii/ui/dataMonitorCollapsed"), false);
    defaults_.insert(QStringLiteral("modbus/ascii/standard/slaveIdStr"), QString::number(::config::Modbus::kDefaultSlaveId));
    defaults_.insert(QStringLiteral("modbus/ascii/standard/startAddrStr"), QString::number(::config::Modbus::kDefaultStandardStartAddress));

    defaults_.insert(kNetworkDebuggerClientIp, QString::fromLatin1(::config::Network::kDefaultDeviceAddress));
    defaults_.insert(kNetworkDebuggerClientPort, ::config::Network::kDefaultNetworkDebuggerPort);
    defaults_.insert(kNetworkDebuggerClientConnectionCollapsed, false);
    defaults_.insert(kNetworkDebuggerClientTrafficAutoScroll, true);
    defaults_.insert(kNetworkDebuggerClientTrafficShowTx, true);
    defaults_.insert(kNetworkDebuggerClientTrafficShowRx, true);
    defaults_.insert(kNetworkDebuggerClientTrafficCollapsed, false);
    defaults_.insert(kNetworkDebuggerClientInputFormat, QStringLiteral("hex"));
    defaults_.insert(kNetworkDebuggerClientInputAutoSend, false);
    defaults_.insert(kNetworkDebuggerClientInputIntervalMs, ::config::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kNetworkDebuggerClientInputCollapsed, false);
    // UDP remote endpoint: the widget reads it without a contains() guard, so
    // it relies on defaults for a clean first-run value (empty host, port 0).
    defaults_.insert(kNetworkDebuggerUdpRemoteIp, QString());
    defaults_.insert(kNetworkDebuggerUdpRemotePort, 0);

    defaults_.insert(kSerialDebuggerBaudRate, QString::fromLatin1(::config::Serial::kDefaultBaudRateText));
    defaults_.insert(kSerialDebuggerDataBits, QString::fromLatin1(::config::Serial::kDefaultDataBitsText));
    defaults_.insert(kSerialDebuggerParity, QString::fromLatin1(::config::Serial::kDefaultParityText));
    defaults_.insert(kSerialDebuggerStopBits, QString::fromLatin1(::config::Serial::kDefaultStopBitsText));
    defaults_.insert(kSerialDebuggerPortName, QString());
    defaults_.insert(kSerialDebuggerConnectionCollapsed, false);
    defaults_.insert(kSerialDebuggerTrafficAutoScroll, true);
    defaults_.insert(kSerialDebuggerTrafficShowTx, true);
    defaults_.insert(kSerialDebuggerTrafficShowRx, true);
    defaults_.insert(kSerialDebuggerTrafficCollapsed, false);
    defaults_.insert(kSerialDebuggerInputFormat, QStringLiteral("hex"));
    defaults_.insert(kSerialDebuggerInputAutoSend, false);
    defaults_.insert(kSerialDebuggerInputIntervalMs, ::config::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kSerialDebuggerInputCollapsed, false);
    defaults_.insert(kSerialDebuggerDtr, false);
    defaults_.insert(kSerialDebuggerRts, false);
}

void SettingsService::load() {
    // Load all keys present in the settings file into values_.
    // Keys absent from the file fall back to defaults_ via value().
    const QStringList allKeys = settings_->allKeys();
    for (const QString& key : allKeys) {
        values_.insert(key, settings_->value(key));
    }

    migrateLegacyKeys();
    scheduleSync();
}

void SettingsService::migrateLegacyKeys() {
    // Phase 9 in-flight migration: copy every legacy-prefixed key to its
    // canonical namespace, then erase the legacy key. Prefix-based (not
    // per-key) so dynamically composed keys — <group>/autoReconnect,
    // <group>/reconnectDelay, widget-internal traffic/input/ui keys — are
    // covered without maintaining a static key inventory.
    bool hasMigration = false;
    const QStringList diskKeys = settings_->allKeys();
    for (const QString& oldKey : diskKeys) {
        for (const auto& entry : kKeyPrefixMigrations) {
            const QString oldPrefix = QLatin1String(entry.oldPrefix);
            if (!oldKey.startsWith(oldPrefix)) {
                continue;
            }
            const QString newKey = QLatin1String(entry.newPrefix) + oldKey.mid(oldPrefix.size());
            // Idempotency: never overwrite a new key the user already has.
            // The stale legacy key is still queued for removal below, so a
            // partially migrated file (crash between the two sync passes)
            // converges to fully cleaned state on the next launch.
            if (!settings_->contains(newKey)) {
                values_.insert(newKey, settings_->value(oldKey));
                dirtyKeys_.insert(newKey);
            }
            keysToRemove_.insert(oldKey);
            hasMigration = true;
            break;
        }
    }
    if (hasMigration) {
        scheduleSync();
    }
}

void SettingsService::scheduleSync() {
    syncTimer_->start();
}

} // namespace infra::config
