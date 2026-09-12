#include <gtest/gtest.h>

#include "core/common/SettingsKeys.h"
#include "infra/config/SettingsService.h"
#include "infra/platform/PathResolver.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <iterator>
#include <functional>

namespace {

// Builds a PathResolver rooted at the supplied sandbox, so tests never
// touch the developer's real application directory (portable-only layout:
// config.ini lives directly in the resolver's application dir).
infra::platform::PathResolver makeIsolatedResolver(const QString& sandboxPath)
{
    return infra::platform::PathResolver(sandboxPath);
}

// RAII guard that backs up and restores the sandbox "config.ini" so each test
// starts from a clean state. Declared BEFORE the SettingsService instance in
// each test so the service (and its debounce QTimer child) is destroyed first,
// preventing a late timer fire from writing after restoration.
class ConfigFileGuard {
public:
    explicit ConfigFileGuard(const QString& configDir) {
        QDir().mkpath(configDir);
        path_ = QDir(configDir).filePath(QStringLiteral("config.ini"));
        if (QFile::exists(path_)) {
            backupPath_ = path_ + QStringLiteral(".test-bak");
            QFile::remove(backupPath_);
            QFile::rename(path_, backupPath_);
        }
    }
    ~ConfigFileGuard() {
        QFile::remove(path_);
        if (!backupPath_.isEmpty() && QFile::exists(backupPath_)) {
            QFile::rename(backupPath_, path_);
        }
    }
    const QString& path() const { return path_; }

private:
    QString path_;
    QString backupPath_;
};

} // namespace

TEST(SettingsService, LoadsValuesFromConfigFile)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    infra::platform::PathResolver pathResolver = makeIsolatedResolver(sandbox.path());
    ConfigFileGuard guard(pathResolver.resolveConfigDir());
    {
        QSettings writer(guard.path(), QSettings::IniFormat);
        writer.setValue(QString::fromLatin1(core::common::settings_keys::kAppThemeMode), QStringLiteral("dark"));
    }

    infra::config::SettingsService service(pathResolver);

    EXPECT_TRUE(service.configFilePath().endsWith(QStringLiteral("config.ini")));
    EXPECT_EQ(service.value(QString::fromLatin1(core::common::settings_keys::kAppThemeMode)).toString().toStdString(),
              std::string("dark"));
}

TEST(SettingsService, SyncPersistsDirtyValues)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    infra::platform::PathResolver pathResolver = makeIsolatedResolver(sandbox.path());
    ConfigFileGuard guard(pathResolver.resolveConfigDir());
    infra::config::SettingsService service(pathResolver);
    service.setValue(QString::fromLatin1(core::common::settings_keys::kAppThemeMode), QStringLiteral("dark"));
    service.sync();

    QSettings reader(guard.path(), QSettings::IniFormat);
    EXPECT_EQ(reader.value(QString::fromLatin1(core::common::settings_keys::kAppThemeMode)).toString().toStdString(),
              std::string("dark"));
}

TEST(SettingsService, LegacySerialBaudRateMigrates)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    infra::platform::PathResolver pathResolver = makeIsolatedResolver(sandbox.path());
    ConfigFileGuard guard(pathResolver.resolveConfigDir());
    {
        QSettings writer(guard.path(), QSettings::IniFormat);
        // The kLegacySerialBaudRate constant was deleted (R2); the legacy key
        // survives only as an on-disk string handled by the prefix engine.
        writer.setValue(QStringLiteral("serial/baudRate"), QStringLiteral("115200"));
    }

    infra::config::SettingsService service(pathResolver);
    service.sync();

    EXPECT_EQ(service.value(QString::fromLatin1(core::common::settings_keys::kModbusRtuBaudRate)).toString().toStdString(),
              std::string("115200"));

    QSettings reader(guard.path(), QSettings::IniFormat);
    EXPECT_FALSE(reader.contains(QStringLiteral("serial/baudRate")));
    EXPECT_EQ(reader.value(QString::fromLatin1(core::common::settings_keys::kModbusRtuBaudRate)).toString().toStdString(),
              std::string("115200"));
}

TEST(SettingsService, InFlightMigration_LegacyNetworkAndSerialKeys)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    infra::platform::PathResolver pathResolver = makeIsolatedResolver(sandbox.path());
    ConfigFileGuard guard(pathResolver.resolveConfigDir());

    // tcp_client: the 11 statically known keys plus the two keys that
    // BaseConnectionWidget composes dynamically for every group.
    const QStringList tcpClientSuffixes = {
        QStringLiteral("ip"), QStringLiteral("port"),
        QStringLiteral("ui/connectionSettingsCollapsed"),
        QStringLiteral("traffic/autoScroll"), QStringLiteral("traffic/showTx"),
        QStringLiteral("traffic/showRx"),
        QStringLiteral("traffic/ui/trafficMonitorCollapsed"),
        QStringLiteral("input/format"), QStringLiteral("input/autoSend"),
        QStringLiteral("input/intervalMs"), QStringLiteral("ui/inputCollapsed"),
        QStringLiteral("autoReconnect"), QStringLiteral("reconnectDelay"),
    };
    // tcp_server / udp: dynamically composed keys only (no static constants
    // ever existed for them — that is exactly why the migration is
    // prefix-based, see GAP-6 in the Phase 9 plan).
    const QStringList dynamicOnlySuffixes = {
        QStringLiteral("autoReconnect"), QStringLiteral("reconnectDelay"),
        QStringLiteral("traffic/autoScroll"), QStringLiteral("traffic/showTx"),
        QStringLiteral("input/format"), QStringLiteral("ui/inputCollapsed"),
    };
    // serial_port: the 16 statically known keys plus the two dynamic ones.
    const QStringList serialSuffixes = {
        QStringLiteral("baudRate"), QStringLiteral("dataBits"),
        QStringLiteral("parity"), QStringLiteral("stopBits"),
        QStringLiteral("portName"),
        QStringLiteral("ui/connectionSettingsCollapsed"),
        QStringLiteral("traffic/autoScroll"), QStringLiteral("traffic/showTx"),
        QStringLiteral("traffic/showRx"),
        QStringLiteral("traffic/ui/trafficMonitorCollapsed"),
        QStringLiteral("input/format"), QStringLiteral("input/autoSend"),
        QStringLiteral("input/intervalMs"), QStringLiteral("ui/inputCollapsed"),
        QStringLiteral("dtr"), QStringLiteral("rts"),
        QStringLiteral("autoReconnect"), QStringLiteral("reconnectDelay"),
    };

    {
        QSettings writer(guard.path(), QSettings::IniFormat);
        // Each legacy value stores its own old key string, so every mapping
        // assertion is self-describing (value must survive byte-for-byte).
        for (const QString& suffix : tcpClientSuffixes) {
            const QString legacyKey = QStringLiteral("tcp_client/") + suffix;
            writer.setValue(legacyKey, legacyKey);
        }
        for (const QString& suffix : dynamicOnlySuffixes) {
            const QString serverKey = QStringLiteral("tcp_server/") + suffix;
            writer.setValue(serverKey, serverKey);
            const QString udpKey = QStringLiteral("udp/") + suffix;
            writer.setValue(udpKey, udpKey);
        }
        writer.setValue(QStringLiteral("udp/remoteIp"), QStringLiteral("udp/remoteIp"));
        writer.setValue(QStringLiteral("udp/remotePort"), 51000);
        for (const QString& suffix : serialSuffixes) {
            const QString legacyKey = QStringLiteral("serial_port/") + suffix;
            writer.setValue(legacyKey, legacyKey);
        }
        // Historical one-off (R1): pre-Modbus-RTU era serial baud key.
        writer.setValue(QStringLiteral("serial/baudRate"), 9600);
    }

    infra::config::SettingsService service(pathResolver);
    service.sync();

    // Snapshot the fully migrated file for the idempotency check below.
    QFile migratedFile(guard.path());
    ASSERT_TRUE(migratedFile.open(QIODevice::ReadOnly));
    const QByteArray afterFirstLoad = migratedFile.readAll();
    migratedFile.close();

    auto expectFamilyMigrated = [&](const QString& legacyPrefix,
                                    const QString& canonicalPrefix,
                                    const QStringList& suffixes) {
        for (const QString& suffix : suffixes) {
            const QString legacyKey = legacyPrefix + suffix;
            const QString canonicalKey = canonicalPrefix + suffix;
            EXPECT_EQ(service.value(canonicalKey).toString().toStdString(),
                      legacyKey.toStdString())
                << canonicalKey.toStdString();
        }
    };
    expectFamilyMigrated(QStringLiteral("tcp_client/"),
                         QStringLiteral("network_debugger/client/"), tcpClientSuffixes);
    expectFamilyMigrated(QStringLiteral("tcp_server/"),
                         QStringLiteral("network_debugger/server/"), dynamicOnlySuffixes);
    expectFamilyMigrated(QStringLiteral("udp/"),
                         QStringLiteral("network_debugger/udp/"), dynamicOnlySuffixes);
    expectFamilyMigrated(QStringLiteral("serial_port/"),
                         QStringLiteral("serial_debugger/"), serialSuffixes);

    EXPECT_EQ(service.value(QStringLiteral("network_debugger/udp/remotePort")).toInt(), 51000);
    EXPECT_EQ(service.value(QStringLiteral("modbus/rtu/serial/baudRate")).toInt(), 9600);

    QSettings reader(guard.path(), QSettings::IniFormat);
    for (const QString& legacyKey : {QStringLiteral("tcp_client/ip"),
                                     QStringLiteral("tcp_server/traffic/autoScroll"),
                                     QStringLiteral("udp/remoteIp"),
                                     QStringLiteral("serial_port/baudRate"),
                                     QStringLiteral("serial/baudRate")}) {
        EXPECT_FALSE(reader.contains(legacyKey)) << legacyKey.toStdString();
    }

    // Idempotency: a second launch over the migrated file performs zero
    // writes — the ini content is byte-identical after the second service
    // lifecycle (load + sync) completes.
    {
        infra::config::SettingsService secondService(pathResolver);
        secondService.sync();
    }
    QFile rereadFile(guard.path());
    ASSERT_TRUE(rereadFile.open(QIODevice::ReadOnly));
    EXPECT_EQ(rereadFile.readAll(), afterFirstLoad);
}

TEST(SettingsService, KeyPrefixMigrations_TableIsPairwiseDisjoint)
{
    using infra::config::SettingsService;
    constexpr std::size_t tableSize = std::size(SettingsService::kKeyPrefixMigrations);
    ASSERT_GT(tableSize, std::size_t{0});
    for (std::size_t i = 0; i < tableSize; ++i) {
        const QString a = QString::fromLatin1(SettingsService::kKeyPrefixMigrations[i].oldPrefix);
        for (std::size_t j = i + 1; j < tableSize; ++j) {
            const QString b = QString::fromLatin1(SettingsService::kKeyPrefixMigrations[j].oldPrefix);
            EXPECT_FALSE(a.startsWith(b))
                << a.toStdString() << " starts with " << b.toStdString();
            EXPECT_FALSE(b.startsWith(a))
                << b.toStdString() << " starts with " << a.toStdString();
        }
    }
}

TEST(SettingsService, DefaultsCoverMigratedKeyNamespaces)
{
    // Fresh sandbox with no config file: every canonical key of the migrated
    // namespaces must fall back to a registered (non-null) default — verifies
    // initializeDefaults() was switched to the new constants (plan item O3).
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    infra::platform::PathResolver pathResolver = makeIsolatedResolver(sandbox.path());
    ConfigFileGuard guard(pathResolver.resolveConfigDir());
    infra::config::SettingsService service(pathResolver);

    using namespace core::common::settings_keys;
    const QStringList canonicalKeys = {
        QString::fromLatin1(kNetworkDebuggerClientIp),
        QString::fromLatin1(kNetworkDebuggerClientPort),
        QString::fromLatin1(kNetworkDebuggerClientConnectionCollapsed),
        QString::fromLatin1(kNetworkDebuggerClientTrafficAutoScroll),
        QString::fromLatin1(kNetworkDebuggerClientTrafficShowTx),
        QString::fromLatin1(kNetworkDebuggerClientTrafficShowRx),
        QString::fromLatin1(kNetworkDebuggerClientTrafficCollapsed),
        QString::fromLatin1(kNetworkDebuggerClientInputFormat),
        QString::fromLatin1(kNetworkDebuggerClientInputAutoSend),
        QString::fromLatin1(kNetworkDebuggerClientInputIntervalMs),
        QString::fromLatin1(kNetworkDebuggerClientInputCollapsed),
        QString::fromLatin1(kNetworkDebuggerUdpRemoteIp),
        QString::fromLatin1(kNetworkDebuggerUdpRemotePort),
        QString::fromLatin1(kSerialDebuggerBaudRate),
        QString::fromLatin1(kSerialDebuggerDataBits),
        QString::fromLatin1(kSerialDebuggerParity),
        QString::fromLatin1(kSerialDebuggerStopBits),
        QString::fromLatin1(kSerialDebuggerPortName),
        QString::fromLatin1(kSerialDebuggerConnectionCollapsed),
        QString::fromLatin1(kSerialDebuggerTrafficAutoScroll),
        QString::fromLatin1(kSerialDebuggerTrafficShowTx),
        QString::fromLatin1(kSerialDebuggerTrafficShowRx),
        QString::fromLatin1(kSerialDebuggerTrafficCollapsed),
        QString::fromLatin1(kSerialDebuggerInputFormat),
        QString::fromLatin1(kSerialDebuggerInputAutoSend),
        QString::fromLatin1(kSerialDebuggerInputIntervalMs),
        QString::fromLatin1(kSerialDebuggerInputCollapsed),
        QString::fromLatin1(kSerialDebuggerDtr),
        QString::fromLatin1(kSerialDebuggerRts),
    };
    for (const QString& key : canonicalKeys) {
        EXPECT_FALSE(service.value(key).isNull()) << key.toStdString();
    }
}
