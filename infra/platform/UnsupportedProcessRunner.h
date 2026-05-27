/**
 * @file UnsupportedProcessRunner.h
 * @brief Provides the default non-supported elevated process launcher.
 */

#pragma once

#include "infra/platform/IPlatformProcessRunner.h"

namespace infra::platform {

class UnsupportedProcessRunner final : public IPlatformProcessRunner {
public:
    ~UnsupportedProcessRunner() noexcept override = default;

    [[nodiscard]] bool startElevated(const QString& executablePath,
                                     const QStringList& arguments,
                                     QString* errorMessage) override;
};

} // namespace infra::platform
