/**
 * @file QtStandardPlatformPaths.h
 * @brief Provides the default writable path implementation backed by QStandardPaths.
 */

#pragma once

#include "infra/platform/IPlatformPaths.h"

namespace infra::platform {

class QtStandardPlatformPaths final : public IPlatformPaths {
public:
    ~QtStandardPlatformPaths() noexcept override = default;

    [[nodiscard]] QString appDataLocation() const override;
    [[nodiscard]] QString appConfigLocation() const override;
    [[nodiscard]] QString tempLocation() const override;
};

} // namespace infra::platform
