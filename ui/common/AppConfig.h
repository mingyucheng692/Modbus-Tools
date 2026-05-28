/**
 * @file AppConfig.h
 * @brief Declares the QSettings-backed application configuration facade.
 */

#pragma once

#include "IAppConfig.h"

namespace ui::common {

class AppConfig final : public IAppConfig {
public:
    AppConfig() = default;
    ~AppConfig() override = default;

    [[nodiscard]] QVariant value(const QString& key, const QVariant& defaultValue = {}) const override;
    [[nodiscard]] bool contains(const QString& key) const override;
    [[nodiscard]] QStringList allKeys() const override;
    [[nodiscard]] QString configFilePath() const override;
    void setValue(const QString& key, const QVariant& value) override;
    void remove(const QString& key) override;
    void sync() override;
};

} // namespace ui::common
