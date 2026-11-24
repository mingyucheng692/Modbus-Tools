// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "SettingsService.h"
#include "core/Config.h"
#include "core/common/SettingsKeys.h"
#include "infra/platform/PathResolver.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <spdlog/spdlog.h>

namespace infra::config {

SettingsService::SettingsService(infra::platform::PathResolver& pathResolver,
                                 QObject* parent)
    : QObject(parent),
      syncTimer_(new QTimer(this)) {
    const QString configDir = pathResolver.resolveConfigDir();
    QDir().mkpath(configDir);
    const QString filePath = QDir(configDir).filePath(QStringLiteral("config.ini"));
    settings_ = std::make_unique<QSettings>(filePath, QSettings::IniFormat);

    syncTimer_->setSingleShot(true);
    syncTimer_->setInterval(::config::Settings::kSyncDebounceMs);
    connect(syncTimer_, &QTimer::timeout, this, &SettingsService::sync);

    initializeDefaults();
    load();
}

SettingsService::~SettingsService() noexcept {
    if (syncTimer_) {
        syncTimer_->stop();
    }
    sync();
}

QVariant SettingsService::value(const QString& key) const {
    if (values_.contains(key)) {
        return values_.value(key);
    }
    return defaults_.value(key);
}

bool SettingsService::contains(const QString& key) const {
    return values_.contains(key) || defaults_.contains(key);
}

void SettingsService::setValue(const QString& key, const QVariant& value) {
    const QVariant currentValue = this->value(key);
    const bool alreadyTracked = values_.contains(key) || dirtyKeys_.contains(key);
    if (currentValue == value && alreadyTracked) {
        return;
    }
    values_.insert(key, value);
    dirtyKeys_.insert(key);
    scheduleSync();
}

QString SettingsService::configFilePath() const {
    return settings_->fileName();
}

void SettingsService::sync() {
    if (dirtyKeys_.isEmpty()) {
        return;
    }

    for (auto it = dirtyKeys_.cbegin(); it != dirtyKeys_.cend(); ++it) {
        settings_->setValue(*it, values_.value(*it, defaults_.value(*it)));
    }
    settings_->sync();
    if (settings_->status() != QSettings::NoError) {
        SPDLOG_WARN("SettingsService: failed to sync '{}', status={}",
                    settings_->fileName().toStdString(),
                    static_cast<int>(settings_->status()));
    }
    dirtyKeys_.clear();
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
    defaults_.insert(kLoggingLevel, static_cast<int>(spdlog::level::info));

    defaults_.insert(kModbusTimeoutMs, ::config::Modbus::kDefaultTimeoutMs);
    defaults_.insert(kModbusRetryCount, ::config::Modbus::kDefaultRetryCount);
    defaults_.insert(kModbusRetryIntervalMs, ::config::Modbus::kDefaultRetryIntervalMs);
    defaults_.insert(kModbusRetryEnabled, ::config::Modbus::kDefaultRetryEnabled);
    defaults_.insert(kModbusAddressBase, 0);

    defaults_.insert(kFrameAnalyzerStartAddr, ::config::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kFrameAnalyzerDecodeMode, 0);
    defaults_.insert(kFrameAnalyzerHistoryCollapsed, false);

    defaults_.insert(kModbusTcpIp, QString::fromLatin1(::config::Network::kDefaultDeviceAddress));
    defaults_.insert(kModbusTcpPort, ::config::Network::kDefaultModbusTcpPort);
    defaults_.insert(kModbusTcpConnectionCollapsed, false);

    defaults_.insert(kModbusRtuBaudRate, QString::fromLatin1(::config::Serial::kDefaultBaudRateText));
    defaults_.insert(kModbusRtuDataBits, QString::fromLatin1(::config::Serial::kDefaultDataBitsText));
    defaults_.insert(kModbusRtuParity, QString::fromLatin1(::config::Serial::kDefaultParityText));
    defaults_.insert(kModbusRtuStopBits, QString::fromLatin1(::config::Serial::kDefaultStopBitsText));
    defaults_.insert(kModbusRtuPortName, QString());
    defaults_.insert(kModbusRtuConnectionCollapsed, false);

    // Modbus ASCII mode defaults (symmetric with RTU serial transport)
    defaults_.insert(kModbusAsciiBaudRate, QString::fromLatin1(::config::Serial::kDefaultBaudRateText));
    defaults_.insert(kModbusAsciiDataBits, QString::fromLatin1(::config::Serial::kDefaultAsciiDataBitsText));
    defaults_.insert(kModbusAsciiParity, QString::fromLatin1(::config::Serial::kDefaultAsciiParityText));
    defaults_.insert(kModbusAsciiStopBits, QString::fromLatin1(::config::Serial::kDefaultStopBitsText));
    defaults_.insert(kModbusAsciiFlowControl, QString::fromLatin1(::config::Serial::kDefaultFlowControlText));
    defaults_.insert(kModbusAsciiPortName, QString());
    defaults_.insert(kModbusAsciiConnectionCollapsed, false);

    // Network Page defaults (Scheme A)
    defaults_.insert(kNetworkProtocol, 0);
    defaults_.insert(kNetworkClientIp, QString::fromLatin1(::config::Network::kDefaultDeviceAddress));
    defaults_.insert(kNetworkClientPort, ::config::Network::kDefaultNetworkPort);
    defaults_.insert(kNetworkClientConnectionCollapsed, false);
    defaults_.insert(kNetworkClientTrafficAutoScroll, true);
    defaults_.insert(kNetworkClientTrafficShowTx, true);
    defaults_.insert(kNetworkClientTrafficShowRx, true);
    defaults_.insert(kNetworkClientInputFormat, QStringLiteral("hex"));
    defaults_.insert(kNetworkClientInputIntervalMs, ::config::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kNetworkClientInputCollapsed, false);
    defaults_.insert(kNetworkUdpRemoteIp, QString());
    defaults_.insert(kNetworkUdpRemotePort, 0);

    // Serial Page defaults (Scheme A)
    defaults_.insert(kSerialBaudRate, QString::fromLatin1(::config::Serial::kDefaultBaudRateText));
    defaults_.insert(kSerialDataBits, QString::fromLatin1(::config::Serial::kDefaultDataBitsText));
    defaults_.insert(kSerialParity, QString::fromLatin1(::config::Serial::kDefaultParityText));
    defaults_.insert(kSerialStopBits, QString::fromLatin1(::config::Serial::kDefaultStopBitsText));
    defaults_.insert(kSerialPortName, QString());
    defaults_.insert(kSerialConnectionCollapsed, false);
    defaults_.insert(kSerialTrafficAutoScroll, true);
    defaults_.insert(kSerialTrafficShowTx, true);
    defaults_.insert(kSerialTrafficShowRx, true);
    defaults_.insert(kSerialInputFormat, QStringLiteral("hex"));
    defaults_.insert(kSerialInputIntervalMs, ::config::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kSerialInputCollapsed, false);
    defaults_.insert(kSerialDtr, false);
    defaults_.insert(kSerialRts, false);

    defaults_.insert(kModbusWorkbenchMode, 0);
    defaults_.insert(kModbusWorkbenchSlaveId, ::config::Modbus::kDefaultSlaveId);
    defaults_.insert(kModbusWorkbenchFunctionCode, 0x03);
    defaults_.insert(kModbusWorkbenchStartAddress, ::config::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kModbusWorkbenchQuantity, 10);
    defaults_.insert(kModbusWorkbenchRadix, 10);
    defaults_.insert(kModbusWorkbenchDataRadix, 10);
    defaults_.insert(kModbusWorkbenchPollIntervalMs, ::config::Polling::kDefaultIntervalMs);
    defaults_.insert(kModbusWorkbenchFc05Value, true);
    defaults_.insert(kModbusWorkbenchFc06Value, QStringLiteral("0"));
    defaults_.insert(kModbusWorkbenchFc0fValue, QStringLiteral("1, 0, 1, 0"));
    defaults_.insert(kModbusWorkbenchFc10Value, QStringLiteral("100, 200"));
    defaults_.insert(kModbusWorkbenchRawText, QString());

    defaults_.insert(kModbusBuilderProtocol, 0);
    defaults_.insert(kModbusBuilderSlaveId, QString::number(::config::Modbus::kDefaultSlaveId));
    defaults_.insert(kModbusBuilderFunctionCode, 2);
    defaults_.insert(kModbusBuilderAddressBase, 0);
    defaults_.insert(kModbusBuilderStartAddress, QStringLiteral("0x0000"));
    defaults_.insert(kModbusBuilderQuantity, 2);
    defaults_.insert(kModbusBuilderWriteData, QStringLiteral("0001 0002"));
    defaults_.insert(kModbusBuilderDataFormat, 0);
}

void SettingsService::load() {
    const QStringList allKeys = settings_->allKeys();
    for (const QString& key : allKeys) {
        values_.insert(key, settings_->value(key));
    }
}

void SettingsService::scheduleSync() {
    syncTimer_->start();
}

} // namespace infra::config
