#include <gtest/gtest.h>

#include "common/SettingsKeys.h"
#include "../../../ui/common/SettingsService.h"
#include "infra/platform/PathResolver.h"

#include <QDir>
#include <QFile>
#include <QSettings>

namespace {

// RAII guard that isolates the real "config.ini" used by SettingsService so
// tests can read/write it without clobbering a developer's real settings.
// The original file is backed up and restored on destruction.
//
// IMPORTANT: declare the guard BEFORE the SettingsService instance in each
// test so the service (and its debounce QTimer child) is destroyed first,
// preventing a late timer fire from writing after restoration.
class ConfigFileGuard {
public:
    ConfigFileGuard() {
        const QString dir = infra::platform::PathResolver::instance().resolveConfigDir();
        QDir().mkpath(dir);
        path_ = QDir(dir).filePath(QStringLiteral("config.ini"));
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
    ConfigFileGuard guard;
    {
        QSettings writer(guard.path(), QSettings::IniFormat);
        writer.setValue(QString::fromLatin1(core::common::settings_keys::kAppThemeMode), QStringLiteral("dark"));
    }

    ui::common::SettingsService service;

    EXPECT_TRUE(service.configFilePath().endsWith(QStringLiteral("config.ini")));
    EXPECT_EQ(service.value(QString::fromLatin1(core::common::settings_keys::kAppThemeMode)).toString().toStdString(),
              std::string("dark"));
}

TEST(SettingsService, SyncPersistsDirtyValues)
{
    ConfigFileGuard guard;
    ui::common::SettingsService service;
    service.setValue(QString::fromLatin1(core::common::settings_keys::kAppThemeMode), QStringLiteral("dark"));
    service.sync();

    QSettings reader(guard.path(), QSettings::IniFormat);
    EXPECT_EQ(reader.value(QString::fromLatin1(core::common::settings_keys::kAppThemeMode)).toString().toStdString(),
              std::string("dark"));
}

TEST(SettingsService, LegacySerialBaudRateMigrates)
{
    ConfigFileGuard guard;
    {
        QSettings writer(guard.path(), QSettings::IniFormat);
        writer.setValue(QString::fromLatin1(core::common::settings_keys::kLegacySerialBaudRate), QStringLiteral("115200"));
    }

    ui::common::SettingsService service;
    service.sync();

    EXPECT_EQ(service.value(QString::fromLatin1(core::common::settings_keys::kModbusRtuBaudRate)).toString().toStdString(),
              std::string("115200"));

    QSettings reader(guard.path(), QSettings::IniFormat);
    EXPECT_FALSE(reader.contains(QString::fromLatin1(core::common::settings_keys::kLegacySerialBaudRate)));
    EXPECT_EQ(reader.value(QString::fromLatin1(core::common::settings_keys::kModbusRtuBaudRate)).toString().toStdString(),
              std::string("115200"));
}
