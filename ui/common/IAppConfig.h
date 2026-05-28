/**
 * @file IAppConfig.h
 * @brief Declares the settings persistence facade used by SettingsService.
 */

#pragma once

#include <QString>
#include <QStringList>
#include <QVariant>

namespace ui::common {

class IAppConfig {
public:
    virtual ~IAppConfig() = default;

    [[nodiscard]] virtual QVariant value(const QString& key, const QVariant& defaultValue = {}) const = 0;
    [[nodiscard]] virtual bool contains(const QString& key) const = 0;
    [[nodiscard]] virtual QStringList allKeys() const = 0;
    [[nodiscard]] virtual QString configFilePath() const = 0;
    virtual void setValue(const QString& key, const QVariant& value) = 0;
    virtual void remove(const QString& key) = 0;
    virtual void sync() = 0;
};

} // namespace ui::common
