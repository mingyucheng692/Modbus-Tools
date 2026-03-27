#include "SettingsService.h"
#include "SettingsKeys.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QTimer>

namespace ui::common {

SettingsService::SettingsService(QObject* parent)
    : QObject(parent),
      syncTimer_(new QTimer(this)) {
    syncTimer_->setSingleShot(true);
    syncTimer_->setInterval(250);
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
    defaults_.insert(kAppNavigationCollapsed, false);
    defaults_.insert(kAppUpdateCheckFrequency, QStringLiteral("startup"));
    defaults_.insert(kAppUpdateLastCheckUtc, QString());
    defaults_.insert(kAppDisclaimerAccepted, false);
    defaults_.insert(kAppMainWindowGeometry, QByteArray());
    defaults_.insert(kAppMainWindowState, QByteArray());

    defaults_.insert(kModbusTimeoutMs, 1000);
    defaults_.insert(kModbusRetryCount, 3);
    defaults_.insert(kModbusRetryIntervalMs, 100);
    defaults_.insert(kModbusRetryEnabled, true);

    defaults_.insert(kFrameAnalyzerStartAddr, 0);
    defaults_.insert(kFrameAnalyzerDecodeMode, 0);
    defaults_.insert(kFrameAnalyzerHistoryCollapsed, false);

    defaults_.insert(kModbusTcpIp, QStringLiteral("127.0.0.1"));
    defaults_.insert(kModbusTcpPort, 502);
    defaults_.insert(kModbusTcpConnectionCollapsed, false);
    defaults_.insert(kModbusTcpStandardSlaveId, 1);
    defaults_.insert(kModbusTcpStandardStartAddr, 0);
    defaults_.insert(kModbusTcpStandardQuantity, 10);
    defaults_.insert(kModbusTcpStandardFormatIndex, 0);
    defaults_.insert(kModbusTcpStandardCollapsed, false);
    defaults_.insert(kModbusTcpRawCollapsed, true);
    defaults_.insert(kModbusTcpTrafficAutoScroll, true);
    defaults_.insert(kModbusTcpTrafficShowTx, true);
    defaults_.insert(kModbusTcpTrafficShowRx, true);
    defaults_.insert(kModbusTcpTrafficCollapsed, false);
    defaults_.insert(kModbusTcpControlEnablePoll, false);
    defaults_.insert(kModbusTcpControlIntervalMs, 1000);
    defaults_.insert(kModbusTcpControlFcIndex, 0);
    defaults_.insert(kModbusTcpControlAddr, 0);
    defaults_.insert(kModbusTcpControlQty, 10);
    defaults_.insert(kModbusTcpDataMonitorCollapsed, false);

    defaults_.insert(kModbusRtuBaudRate, QStringLiteral("9600"));
    defaults_.insert(kModbusRtuDataBits, QStringLiteral("8"));
    defaults_.insert(kModbusRtuParity, QStringLiteral("None"));
    defaults_.insert(kModbusRtuStopBits, QStringLiteral("1"));
    defaults_.insert(kModbusRtuPortName, QString());
    defaults_.insert(kModbusRtuConnectionCollapsed, false);
    defaults_.insert(kModbusRtuStandardSlaveId, 1);
    defaults_.insert(kModbusRtuStandardStartAddr, 0);
    defaults_.insert(kModbusRtuStandardQuantity, 10);
    defaults_.insert(kModbusRtuStandardFormatIndex, 0);
    defaults_.insert(kModbusRtuStandardCollapsed, false);
    defaults_.insert(kModbusRtuRawCollapsed, true);
    defaults_.insert(kModbusRtuTrafficAutoScroll, true);
    defaults_.insert(kModbusRtuTrafficShowTx, true);
    defaults_.insert(kModbusRtuTrafficShowRx, true);
    defaults_.insert(kModbusRtuTrafficCollapsed, false);
    defaults_.insert(kModbusRtuControlEnablePoll, false);
    defaults_.insert(kModbusRtuControlIntervalMs, 1000);
    defaults_.insert(kModbusRtuControlFcIndex, 0);
    defaults_.insert(kModbusRtuControlAddr, 0);
    defaults_.insert(kModbusRtuControlQty, 10);
    defaults_.insert(kModbusRtuDataMonitorCollapsed, false);

    defaults_.insert(kTcpClientIp, QStringLiteral("127.0.0.1"));
    defaults_.insert(kTcpClientPort, 8080);
    defaults_.insert(kTcpClientConnectionCollapsed, false);
    defaults_.insert(kTcpClientTrafficAutoScroll, true);
    defaults_.insert(kTcpClientTrafficShowTx, true);
    defaults_.insert(kTcpClientTrafficShowRx, true);
    defaults_.insert(kTcpClientTrafficCollapsed, false);
    defaults_.insert(kTcpClientInputFormat, QStringLiteral("hex"));
    defaults_.insert(kTcpClientInputAutoSend, false);
    defaults_.insert(kTcpClientInputIntervalMs, 1000);
    defaults_.insert(kTcpClientInputCollapsed, false);

    defaults_.insert(kSerialPortBaudRate, QStringLiteral("9600"));
    defaults_.insert(kSerialPortDataBits, QStringLiteral("8"));
    defaults_.insert(kSerialPortParity, QStringLiteral("None"));
    defaults_.insert(kSerialPortStopBits, QStringLiteral("1"));
    defaults_.insert(kSerialPortPortName, QString());
    defaults_.insert(kSerialPortConnectionCollapsed, false);
    defaults_.insert(kSerialPortTrafficAutoScroll, true);
    defaults_.insert(kSerialPortTrafficShowTx, true);
    defaults_.insert(kSerialPortTrafficShowRx, true);
    defaults_.insert(kSerialPortTrafficCollapsed, false);
    defaults_.insert(kSerialPortInputFormat, QStringLiteral("hex"));
    defaults_.insert(kSerialPortInputAutoSend, false);
    defaults_.insert(kSerialPortInputIntervalMs, 1000);
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
