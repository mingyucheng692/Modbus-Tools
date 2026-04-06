#include "SettingsService.h"
#include "SettingsKeys.h"
#include "AppConstants.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QTimer>

namespace ui::common {

SettingsService::SettingsService(QObject* parent)
    : QObject(parent),
      syncTimer_(new QTimer(this)) {
    syncTimer_->setSingleShot(true);
    syncTimer_->setInterval(app::constants::Constants::Settings::kSyncDebounceMs);
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
    return QDir(QCoreApplication::applicationDirPath()).filePath("config.ini");
}

void SettingsService::sync() {
    if (dirtyKeys_.isEmpty() && keysToRemove_.isEmpty()) {
        return;
    }

    QSettings settings(configFilePath(), QSettings::IniFormat);
    for (auto it = dirtyKeys_.cbegin(); it != dirtyKeys_.cend(); ++it) {
        settings.setValue(*it, values_.value(*it, defaults_.value(*it)));
        loadedKeys_.insert(*it);
    }
    for (auto it = keysToRemove_.cbegin(); it != keysToRemove_.cend(); ++it) {
        settings.remove(*it);
        loadedKeys_.remove(*it);
    }
    settings.sync();
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

    defaults_.insert(kModbusTimeoutMs, app::constants::Constants::Modbus::kDefaultTimeoutMs);
    defaults_.insert(kModbusRetryCount, app::constants::Constants::Modbus::kDefaultRetryCount);
    defaults_.insert(kModbusRetryIntervalMs, app::constants::Constants::Modbus::kDefaultRetryIntervalMs);
    defaults_.insert(kModbusRetryEnabled, app::constants::Constants::Modbus::kDefaultRetryEnabled);

    defaults_.insert(kFrameAnalyzerStartAddr, app::constants::Constants::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kFrameAnalyzerDecodeMode, app::constants::Constants::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kFrameAnalyzerHistoryCollapsed, false);

    defaults_.insert(kModbusTcpIp, QString::fromLatin1(app::constants::Constants::Network::kLoopbackAddress));
    defaults_.insert(kModbusTcpPort, app::constants::Constants::Network::kDefaultModbusTcpPort);
    defaults_.insert(kModbusTcpConnectionCollapsed, false);
    defaults_.insert(kModbusTcpStandardSlaveId, app::constants::Constants::Modbus::kDefaultSlaveId);
    defaults_.insert(kModbusTcpStandardStartAddr, app::constants::Constants::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kModbusTcpStandardQuantity, app::constants::Constants::Modbus::kDefaultStandardQuantity);
    defaults_.insert(kModbusTcpStandardFormatIndex, app::constants::Constants::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kModbusTcpStandardCollapsed, false);
    defaults_.insert(kModbusTcpRawCollapsed, true);
    defaults_.insert(kModbusTcpTrafficAutoScroll, true);
    defaults_.insert(kModbusTcpTrafficShowTx, true);
    defaults_.insert(kModbusTcpTrafficShowRx, true);
    defaults_.insert(kModbusTcpTrafficCollapsed, false);
    defaults_.insert(kModbusTcpControlEnablePoll, false);
    defaults_.insert(kModbusTcpControlIntervalMs, app::constants::Constants::Modbus::kDefaultControlIntervalMs);
    defaults_.insert(kModbusTcpControlFcIndex, app::constants::Constants::Modbus::kDefaultControlFunctionIndex);
    defaults_.insert(kModbusTcpControlAddr, app::constants::Constants::Modbus::kDefaultControlAddress);
    defaults_.insert(kModbusTcpControlQty, app::constants::Constants::Modbus::kDefaultControlQuantity);
    defaults_.insert(kModbusTcpDataMonitorCollapsed, false);

    defaults_.insert(kModbusRtuBaudRate, QString::fromLatin1(app::constants::Constants::Serial::kDefaultBaudRateText));
    defaults_.insert(kModbusRtuDataBits, QString::fromLatin1(app::constants::Constants::Serial::kDefaultDataBitsText));
    defaults_.insert(kModbusRtuParity, QString::fromLatin1(app::constants::Constants::Serial::kDefaultParityText));
    defaults_.insert(kModbusRtuStopBits, QString::fromLatin1(app::constants::Constants::Serial::kDefaultStopBitsText));
    defaults_.insert(kModbusRtuPortName, QString());
    defaults_.insert(kModbusRtuConnectionCollapsed, false);
    defaults_.insert(kModbusRtuStandardSlaveId, app::constants::Constants::Modbus::kDefaultSlaveId);
    defaults_.insert(kModbusRtuStandardStartAddr, app::constants::Constants::Modbus::kDefaultStandardStartAddress);
    defaults_.insert(kModbusRtuStandardQuantity, app::constants::Constants::Modbus::kDefaultStandardQuantity);
    defaults_.insert(kModbusRtuStandardFormatIndex, app::constants::Constants::Modbus::kDefaultStandardFormatIndex);
    defaults_.insert(kModbusRtuStandardCollapsed, false);
    defaults_.insert(kModbusRtuRawCollapsed, true);
    defaults_.insert(kModbusRtuTrafficAutoScroll, true);
    defaults_.insert(kModbusRtuTrafficShowTx, true);
    defaults_.insert(kModbusRtuTrafficShowRx, true);
    defaults_.insert(kModbusRtuTrafficCollapsed, false);
    defaults_.insert(kModbusRtuControlEnablePoll, false);
    defaults_.insert(kModbusRtuControlIntervalMs, app::constants::Constants::Modbus::kDefaultControlIntervalMs);
    defaults_.insert(kModbusRtuControlFcIndex, app::constants::Constants::Modbus::kDefaultControlFunctionIndex);
    defaults_.insert(kModbusRtuControlAddr, app::constants::Constants::Modbus::kDefaultControlAddress);
    defaults_.insert(kModbusRtuControlQty, app::constants::Constants::Modbus::kDefaultControlQuantity);
    defaults_.insert(kModbusRtuDataMonitorCollapsed, false);

    defaults_.insert(kTcpClientIp, QString::fromLatin1(app::constants::Constants::Network::kLoopbackAddress));
    defaults_.insert(kTcpClientPort, app::constants::Constants::Network::kDefaultGenericTcpPort);
    defaults_.insert(kTcpClientConnectionCollapsed, false);
    defaults_.insert(kTcpClientTrafficAutoScroll, true);
    defaults_.insert(kTcpClientTrafficShowTx, true);
    defaults_.insert(kTcpClientTrafficShowRx, true);
    defaults_.insert(kTcpClientTrafficCollapsed, false);
    defaults_.insert(kTcpClientInputFormat, QStringLiteral("hex"));
    defaults_.insert(kTcpClientInputAutoSend, false);
    defaults_.insert(kTcpClientInputIntervalMs, app::constants::Constants::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kTcpClientInputCollapsed, false);

    defaults_.insert(kSerialPortBaudRate, QString::fromLatin1(app::constants::Constants::Serial::kDefaultBaudRateText));
    defaults_.insert(kSerialPortDataBits, QString::fromLatin1(app::constants::Constants::Serial::kDefaultDataBitsText));
    defaults_.insert(kSerialPortParity, QString::fromLatin1(app::constants::Constants::Serial::kDefaultParityText));
    defaults_.insert(kSerialPortStopBits, QString::fromLatin1(app::constants::Constants::Serial::kDefaultStopBitsText));
    defaults_.insert(kSerialPortPortName, QString());
    defaults_.insert(kSerialPortConnectionCollapsed, false);
    defaults_.insert(kSerialPortTrafficAutoScroll, true);
    defaults_.insert(kSerialPortTrafficShowTx, true);
    defaults_.insert(kSerialPortTrafficShowRx, true);
    defaults_.insert(kSerialPortTrafficCollapsed, false);
    defaults_.insert(kSerialPortInputFormat, QStringLiteral("hex"));
    defaults_.insert(kSerialPortInputAutoSend, false);
    defaults_.insert(kSerialPortInputIntervalMs, app::constants::Constants::GenericIo::kDefaultInputIntervalMs);
    defaults_.insert(kSerialPortInputCollapsed, false);
    defaults_.insert(kSerialPortDtr, false);
    defaults_.insert(kSerialPortRts, false);
}

void SettingsService::load() {
    QSettings settings(configFilePath(), QSettings::IniFormat);
    for (auto it = defaults_.cbegin(); it != defaults_.cend(); ++it) {
        if (settings.contains(it.key())) {
            loadedKeys_.insert(it.key());
            values_.insert(it.key(), settings.value(it.key(), it.value()));
        } else {
            values_.insert(it.key(), it.value());
            dirtyKeys_.insert(it.key());
        }
    }

    using namespace settings_keys;
    if (settings.contains(kLegacySerialBaudRate) && !settings.contains(kModbusRtuBaudRate)) {
        values_.insert(kModbusRtuBaudRate, settings.value(kLegacySerialBaudRate));
        dirtyKeys_.insert(kModbusRtuBaudRate);
        keysToRemove_.insert(kLegacySerialBaudRate);
    }
    scheduleSync();
}

void SettingsService::scheduleSync() {
    syncTimer_->start();
}

} // namespace ui::common
