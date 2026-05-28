/**
 * @file AppConfig.cpp
 * @brief Implements the QSettings-backed application configuration facade.
 */

#include "AppConfig.h"

#include "infra/platform/PathResolver.h"

#include <QDir>
#include <QSettings>

namespace {

QString resolveAppConfigFilePath()
{
    return QDir(infra::platform::PathResolver::instance().resolveConfigDir()).filePath(QStringLiteral("config.ini"));
}

} // namespace

namespace ui::common {

QVariant AppConfig::value(const QString& key, const QVariant& defaultValue) const
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    return settings.value(key, defaultValue);
}

bool AppConfig::contains(const QString& key) const
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    return settings.contains(key);
}

QStringList AppConfig::allKeys() const
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    return settings.allKeys();
}

QString AppConfig::configFilePath() const
{
    return resolveAppConfigFilePath();
}

void AppConfig::setValue(const QString& key, const QVariant& value)
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    settings.setValue(key, value);
}

void AppConfig::remove(const QString& key)
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    settings.remove(key);
}

void AppConfig::sync()
{
    QSettings settings(configFilePath(), QSettings::IniFormat);
    settings.sync();
}

} // namespace ui::common
