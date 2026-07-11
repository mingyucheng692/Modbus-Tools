/**
 * @file QtStandardPlatformPaths.h
 * @brief Provides the default writable path implementation backed by QStandardPaths.
 */

#pragma once

#include "infra/platform/IPlatformPaths.h"
#include <QStandardPaths>

namespace infra::platform {

class QtStandardPlatformPaths final : public IPlatformPaths {
public:
    ~QtStandardPlatformPaths() noexcept override = default;

    [[nodiscard]] QString appDataLocation() const override {
        return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    }

    [[nodiscard]] QString appConfigLocation() const override {
        return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    }

    [[nodiscard]] QString tempLocation() const override {
        return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    }
};

} // namespace infra::platform
