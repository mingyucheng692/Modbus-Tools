#include <gtest/gtest.h>

#include "common/SettingsKeys.h"
#include "../../../ui/common/SettingsService.h"
#include "infra/platform/PathResolver.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
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

    ui::common::SettingsService service(pathResolver);

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
    ui::common::SettingsService service(pathResolver);
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
        writer.setValue(QString::fromLatin1(core::common::settings_keys::kLegacySerialBaudRate), QStringLiteral("115200"));
    }

    ui::common::SettingsService service(pathResolver);
    service.sync();

    EXPECT_EQ(service.value(QString::fromLatin1(core::common::settings_keys::kModbusRtuBaudRate)).toString().toStdString(),
              std::string("115200"));

    QSettings reader(guard.path(), QSettings::IniFormat);
    EXPECT_FALSE(reader.contains(QString::fromLatin1(core::common::settings_keys::kLegacySerialBaudRate)));
    EXPECT_EQ(reader.value(QString::fromLatin1(core::common::settings_keys::kModbusRtuBaudRate)).toString().toStdString(),
              std::string("115200"));
}