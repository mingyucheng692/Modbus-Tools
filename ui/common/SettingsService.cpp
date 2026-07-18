/**
 * @file SettingsService.cpp
 * @brief Implementation of SettingsService.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "SettingsService.h"
#include "common/SettingsKeys.h"
#include "Config.h"
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

namespace ui::common {

SettingsService::SettingsService(infra::platform::PathResolver& pathResolver,
                                 QObject* parent)
    : QObject(parent),
      settings_(std::make_unique<QSettings>(resolveConfigFilePath(pathResolver), QSettings::IniFormat)),
      syncTimer_(new QTimer(this)),
      pathResolver_(pathResolver) {
    syncTimer_->setSingleShot(true);
    syncTimer_->setInterval(config::Settings::kSyncDebounceMs);
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

    defaults_.insert(kModbusTimeoutMs, config::Modbus::kDefaultTimeoutMs);
    defaults_.insert(kModbusRetryCount, config::Modbus::kDefaultRetryCount);
    defaults_.insert(kModbusRetryIntervalMs, config::Modbus::kDefaultRetryIntervalMs);
    defaults_.insert(kModbusRetryEnabled, config::Modbus::kDefaultRetryEnabled);

    defaults_.insert(kFrameAnalyzerStartAddr, config::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kFrameAnalyzerDecodeMode, config::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kFrameAnalyzerHistoryCollapsed, false);

    defaults_.insert(kModbusTcpIp, QString::fromLatin1(config::Network::kDefaultDeviceAddress));
    defaults_.insert(kModbusTcpPort, config::Network::kDefaultModbusTcpPort);
    defaults_.insert(kModbusTcpConnectionCollapsed, false);
    defaults_.insert(kModbusTcpStandardSlaveId, config::Modbus::kDefaultSlaveId);
    defaults_.insert(kModbusTcpStandardStartAddr, config::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kModbusTcpStandardQuantity, config::Modbus::kDefaultStandardQuantity);
    defaults_.insert(kModbusTcpStandardFormatIndex, config::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kModbusTcpStandardCollapsed, false);
    defaults_.insert(kModbusTcpRawCollapsed, true);
    defaults_.insert(kModbusTcpTrafficAutoScroll, true);
    defaults_.insert(kModbusTcpTrafficShowTx, true);
    defaults_.insert(kModbusTcpTrafficShowRx, true);
    defaults_.insert(kModbusTcpTrafficCollapsed, false);
    defaults_.insert(kModbusTcpControlEnablePoll, false);
    defaults_.insert(kModbusTcpControlIntervalMs, config::Modbus::kDefaultControlIntervalMs);
    defaults_.insert(kModbusTcpControlFcIndex, config::Modbus::kDefaultControlFunctionIndex);
    defaults_.insert(kModbusTcpControlAddr, config::Modbus::kDefaultControlAddress);
    defaults_.insert(kModbusTcpControlQty, config::Modbus::kDefaultControlQuantity);
    defaults_.insert(kModbusTcpDataMonitorCollapsed, false);

    defaults_.insert(kModbusRtuBaudRate, QString::fromLatin1(config::Serial::kDefaultBaudRateText));
    defaults_.insert(kModbusRtuDataBits, QString::fromLatin1(config::Serial::kDefaultDataBitsText));
    defaults_.insert(kModbusRtuParity, QString::fromLatin1(config::Serial::kDefaultParityText));
    defaults_.insert(kModbusRtuStopBits, QString::fromLatin1(config::Serial::kDefaultStopBitsText));
    defaults_.insert(kModbusRtuPortName, QString());
    defaults_.insert(kModbusRtuConnectionCollapsed, false);
    defaults_.insert(kModbusRtuStandardSlaveId, config::Modbus::kDefaultSlaveId);
    defaults_.insert(kModbusRtuStandardStartAddr, config::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kModbusRtuStandardQuantity, config::Modbus::kDefaultStandardQuantity);
    defaults_.insert(kModbusRtuStandardFormatIndex, config::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kModbusRtuStandardCollapsed, false);
    defaults_.insert(kModbusRtuRawCollapsed, true);
    defaults_.insert(kModbusRtuTrafficAutoScroll, true);
    defaults_.insert(kModbusRtuTrafficShowTx, true);
    defaults_.insert(kModbusRtuTrafficShowRx, true);
    defaults_.insert(kModbusRtuTrafficCollapsed, false);
    defaults_.insert(kModbusRtuControlEnablePoll, false);
    defaults_.insert(kModbusRtuControlIntervalMs, config::Modbus::kDefaultControlIntervalMs);
    defaults_.insert(kModbusRtuControlFcIndex, config::Modbus::kDefaultControlFunctionIndex);
    defaults_.insert(kModbusRtuControlAddr, config::Modbus::kDefaultControlAddress);
    defaults_.insert(kModbusRtuControlQty, config::Modbus::kDefaultControlQuantity);
    defaults_.insert(kModbusRtuDataMonitorCollapsed, false);

    // New string-based keys for smart hex/dec input support
    defaults_.insert(QString(kModbusTcpStandardSlaveId) + "Str", QString::number(config::Modbus::kDefaultSlaveId));
    defaults_.insert(QString(kModbusTcpStandardStartAddr) + "Str", QString::number(config::Modbus::kDefaultStandardStartAddress));
    defaults_.insert(QString(kModbusRtuStandardSlaveId) + "Str", QString::number(config::Modbus::kDefaultSlaveId));
    defaults_.insert(QString(kModbusRtuStandardStartAddr) + "Str", QString::number(config::Modbus::kDefaultStandardStartAddress));

    defaults_.insert(kTcpClientIp, QString::fromLatin1(config::Network::kDefaultDeviceAddress));
    defaults_.insert(kTcpClientPort, config::Network::kDefaultGenericTcpPort);
    defaults_.insert(kTcpClientConnectionCollapsed, false);
    defaults_.insert(kTcpClientTrafficAutoScroll, true);
    defaults_.insert(kTcpClientTrafficShowTx, true);
    defaults_.insert(kTcpClientTrafficShowRx, true);
    defaults_.insert(kTcpClientTrafficCollapsed, false);
    defaults_.insert(kTcpClientInputFormat, QStringLiteral("hex"));
    defaults_.insert(kTcpClientInputAutoSend, false);
    defaults_.insert(kTcpClientInputIntervalMs, config::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kTcpClientInputCollapsed, false);

    defaults_.insert(kSerialPortBaudRate, QString::fromLatin1(config::Serial::kDefaultBaudRateText));
    defaults_.insert(kSerialPortDataBits, QString::fromLatin1(config::Serial::kDefaultDataBitsText));
    defaults_.insert(kSerialPortParity, QString::fromLatin1(config::Serial::kDefaultParityText));
    defaults_.insert(kSerialPortStopBits, QString::fromLatin1(config::Serial::kDefaultStopBitsText));
    defaults_.insert(kSerialPortPortName, QString());
    defaults_.insert(kSerialPortConnectionCollapsed, false);
    defaults_.insert(kSerialPortTrafficAutoScroll, true);
    defaults_.insert(kSerialPortTrafficShowTx, true);
    defaults_.insert(kSerialPortTrafficShowRx, true);
    defaults_.insert(kSerialPortTrafficCollapsed, false);
    defaults_.insert(kSerialPortInputFormat, QStringLiteral("hex"));
    defaults_.insert(kSerialPortInputAutoSend, false);
    defaults_.insert(kSerialPortInputIntervalMs, config::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kSerialPortInputCollapsed, false);
    defaults_.insert(kSerialPortDtr, false);
    defaults_.insert(kSerialPortRts, false);
}

void SettingsService::load() {
    // Load all keys present in the settings file into values_.
    // Keys absent from the file fall back to defaults_ via value().
    const QStringList allKeys = settings_->allKeys();
    for (const QString& key : allKeys) {
        values_.insert(key, settings_->value(key));
    }

    using namespace core::common::settings_keys;
    if (settings_->contains(kLegacySerialBaudRate) && !settings_->contains(kModbusRtuBaudRate)) {
        values_.insert(kModbusRtuBaudRate, settings_->value(kLegacySerialBaudRate));
        dirtyKeys_.insert(kModbusRtuBaudRate);
        keysToRemove_.insert(kLegacySerialBaudRate);
    }
    scheduleSync();
}

void SettingsService::scheduleSync() {
    syncTimer_->start();
}

} // namespace ui::common
