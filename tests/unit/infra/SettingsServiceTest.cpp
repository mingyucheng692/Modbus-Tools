// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "core/common/SettingsKeys.h"
#include "infra/config/SettingsService.h"
#include "infra/platform/PathResolver.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QTimer>
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

TEST(SettingsService, DefaultsCoverNetworkAndSerialNamespaces)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    infra::platform::PathResolver pathResolver = makeIsolatedResolver(sandbox.path());
    ConfigFileGuard guard(pathResolver.resolveConfigDir());
    infra::config::SettingsService service(pathResolver);

    using namespace core::common::settings_keys;
    const QStringList canonicalKeys = {
        QString::fromLatin1(kNetworkProtocol),
        QString::fromLatin1(kNetworkClientIp),
        QString::fromLatin1(kNetworkClientPort),
        QString::fromLatin1(kNetworkClientConnectionCollapsed),
        QString::fromLatin1(kNetworkClientTrafficAutoScroll),
        QString::fromLatin1(kNetworkClientTrafficShowTx),
        QString::fromLatin1(kNetworkClientTrafficShowRx),
        QString::fromLatin1(kNetworkClientInputFormat),
        QString::fromLatin1(kNetworkClientInputIntervalMs),
        QString::fromLatin1(kNetworkClientInputCollapsed),
        QString::fromLatin1(kNetworkUdpRemoteIp),
        QString::fromLatin1(kNetworkUdpRemotePort),
        QString::fromLatin1(kSerialBaudRate),
        QString::fromLatin1(kSerialDataBits),
        QString::fromLatin1(kSerialParity),
        QString::fromLatin1(kSerialStopBits),
        QString::fromLatin1(kSerialPortName),
        QString::fromLatin1(kSerialConnectionCollapsed),
        QString::fromLatin1(kSerialTrafficAutoScroll),
        QString::fromLatin1(kSerialTrafficShowTx),
        QString::fromLatin1(kSerialTrafficShowRx),
        QString::fromLatin1(kSerialInputFormat),
        QString::fromLatin1(kSerialInputIntervalMs),
        QString::fromLatin1(kSerialInputCollapsed),
        QString::fromLatin1(kSerialDtr),
        QString::fromLatin1(kSerialRts),
    };
    for (const QString& key : canonicalKeys) {
        EXPECT_FALSE(service.value(key).isNull()) << key.toStdString();
    }
}

TEST(SettingsService, LoadWithoutPendingSyncDoesNotScheduleSyncOrWriteFile)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    infra::platform::PathResolver pathResolver = makeIsolatedResolver(sandbox.path());
    ConfigFileGuard guard(pathResolver.resolveConfigDir());
    infra::config::SettingsService service(pathResolver);

    const auto* syncTimer = service.findChild<QTimer*>();
    ASSERT_NE(syncTimer, nullptr);
    EXPECT_FALSE(syncTimer->isActive());
    EXPECT_FALSE(QFile::exists(guard.path()));
}

TEST(SettingsService, DestructorFlushesPendingDirtyValuesWithoutExplicitSync)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    infra::platform::PathResolver pathResolver = makeIsolatedResolver(sandbox.path());
    ConfigFileGuard guard(pathResolver.resolveConfigDir());
    {
        infra::config::SettingsService service(pathResolver);
        service.setValue(QString::fromLatin1(core::common::settings_keys::kAppThemeMode),
                         QStringLiteral("light"));
        const auto* syncTimer = service.findChild<QTimer*>();
        ASSERT_NE(syncTimer, nullptr);
        EXPECT_TRUE(syncTimer->isActive());
    }

    QSettings reader(guard.path(), QSettings::IniFormat);
    EXPECT_EQ(reader.value(QString::fromLatin1(core::common::settings_keys::kAppThemeMode)).toString().toStdString(),
              std::string("light"));
}

TEST(SettingsService, DefaultsCoverModbusAsciiNamespace)
{
    QTemporaryDir sandbox;
    ASSERT_TRUE(sandbox.isValid());

    infra::platform::PathResolver pathResolver = makeIsolatedResolver(sandbox.path());
    ConfigFileGuard guard(pathResolver.resolveConfigDir());
    infra::config::SettingsService service(pathResolver);

    using namespace core::common::settings_keys;
    for (const char* key : {
             kModbusAsciiBaudRate,
             kModbusAsciiDataBits,
             kModbusAsciiParity,
             kModbusAsciiStopBits,
             kModbusAsciiFlowControl,
             kModbusAsciiPortName,
             kModbusAsciiConnectionCollapsed,
         }) {
        EXPECT_FALSE(service.value(QString::fromLatin1(key)).isNull()) << key;
    }

    EXPECT_EQ(service.value(QString::fromLatin1(kModbusAsciiDataBits)).toString(), QStringLiteral("7"));
    EXPECT_EQ(service.value(QString::fromLatin1(kModbusAsciiParity)).toString(), QStringLiteral("even"));
    EXPECT_EQ(service.value(QString::fromLatin1(kModbusAsciiFlowControl)).toString(), QStringLiteral("none"));

    for (const char* key : {
             kLoggingLevel,
             kModbusAddressBase,
             kModbusWorkbenchMode,
             kModbusWorkbenchSlaveId,
             kModbusWorkbenchFunctionCode,
             kModbusWorkbenchStartAddress,
             kModbusWorkbenchQuantity,
             kModbusWorkbenchRadix,
             kModbusWorkbenchDataRadix,
             kModbusWorkbenchPollIntervalMs,
             kModbusWorkbenchFc05Value,
             kModbusWorkbenchFc06Value,
             kModbusWorkbenchFc0fValue,
             kModbusWorkbenchFc10Value,
             kModbusWorkbenchRawText,
             kModbusBuilderProtocol,
             kModbusBuilderSlaveId,
             kModbusBuilderFunctionCode,
             kModbusBuilderAddressBase,
             kModbusBuilderStartAddress,
             kModbusBuilderQuantity,
             kModbusBuilderWriteData,
             kModbusBuilderDataFormat,
         }) {
        EXPECT_FALSE(service.value(QString::fromLatin1(key)).isNull()) << key;
    }
}
