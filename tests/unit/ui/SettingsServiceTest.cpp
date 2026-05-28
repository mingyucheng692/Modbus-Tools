#include <gtest/gtest.h>

#include "../../../ui/common/IAppConfig.h"
#include "../../../ui/common/SettingsKeys.h"
#include "../../../ui/common/SettingsService.h"

#include <QHash>
#include <memory>

namespace {

class FakeAppConfig final : public ui::common::IAppConfig {
public:
    [[nodiscard]] QVariant value(const QString& key, const QVariant& defaultValue = {}) const override
    {
        return values_.value(key, defaultValue);
    }

    [[nodiscard]] bool contains(const QString& key) const override
    {
        return values_.contains(key);
    }

    [[nodiscard]] QStringList allKeys() const override
    {
        return values_.keys();
    }

    [[nodiscard]] QString configFilePath() const override
    {
        return configFilePath_;
    }

    void setValue(const QString& key, const QVariant& value) override
    {
        values_.insert(key, value);
        writtenKeys_.append(key);
    }

    void remove(const QString& key) override
    {
        values_.remove(key);
        removedKeys_.append(key);
    }

    void sync() override
    {
        ++syncCount_;
    }

    QString configFilePath_ = QStringLiteral("fake-config.ini");
    QHash<QString, QVariant> values_;
    QStringList writtenKeys_;
    QStringList removedKeys_;
    int syncCount_ = 0;
};

} // namespace

TEST(SettingsService, LoadsValuesThroughAppConfigFacade)
{
    auto fakeAppConfig = std::make_unique<FakeAppConfig>();
    fakeAppConfig->configFilePath_ = QStringLiteral("settings-under-test.ini");
    fakeAppConfig->values_.insert(QString::fromLatin1(ui::common::settings_keys::kAppThemeMode), QStringLiteral("dark"));
    FakeAppConfig* fakeAppConfigPtr = fakeAppConfig.get();

    ui::common::SettingsService service(nullptr, std::move(fakeAppConfig));

    EXPECT_EQ(service.configFilePath().toStdString(), std::string("settings-under-test.ini"));
    EXPECT_EQ(service.value(QString::fromLatin1(ui::common::settings_keys::kAppThemeMode)).toString().toStdString(),
              std::string("dark"));
    EXPECT_TRUE(fakeAppConfigPtr->writtenKeys_.isEmpty());
}

TEST(SettingsService, SyncPersistsDirtyValuesThroughAppConfigFacade)
{
    auto fakeAppConfig = std::make_unique<FakeAppConfig>();
    FakeAppConfig* fakeAppConfigPtr = fakeAppConfig.get();

    ui::common::SettingsService service(nullptr, std::move(fakeAppConfig));
    service.setValue(QString::fromLatin1(ui::common::settings_keys::kAppThemeMode), QStringLiteral("dark"));
    service.sync();

    EXPECT_EQ(fakeAppConfigPtr->values_.value(QString::fromLatin1(ui::common::settings_keys::kAppThemeMode)).toString().toStdString(),
              std::string("dark"));
    EXPECT_EQ(fakeAppConfigPtr->syncCount_, 1);
    EXPECT_TRUE(fakeAppConfigPtr->writtenKeys_.contains(QString::fromLatin1(ui::common::settings_keys::kAppThemeMode)));
}

TEST(SettingsService, LegacySerialBaudRateMigratesThroughAppConfigFacade)
{
    auto fakeAppConfig = std::make_unique<FakeAppConfig>();
    fakeAppConfig->values_.insert(QString::fromLatin1(ui::common::settings_keys::kLegacySerialBaudRate), QStringLiteral("115200"));
    FakeAppConfig* fakeAppConfigPtr = fakeAppConfig.get();

    ui::common::SettingsService service(nullptr, std::move(fakeAppConfig));
    service.sync();

    EXPECT_EQ(service.value(QString::fromLatin1(ui::common::settings_keys::kModbusRtuBaudRate)).toString().toStdString(),
              std::string("115200"));
    EXPECT_FALSE(fakeAppConfigPtr->values_.contains(QString::fromLatin1(ui::common::settings_keys::kLegacySerialBaudRate)));
    EXPECT_EQ(fakeAppConfigPtr->values_.value(QString::fromLatin1(ui::common::settings_keys::kModbusRtuBaudRate)).toString().toStdString(),
              std::string("115200"));
    EXPECT_TRUE(fakeAppConfigPtr->removedKeys_.contains(QString::fromLatin1(ui::common::settings_keys::kLegacySerialBaudRate)));
}
