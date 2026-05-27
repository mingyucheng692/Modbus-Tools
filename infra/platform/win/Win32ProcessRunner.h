/**
 * @file Win32ProcessRunner.h
 * @brief Declares the Windows elevated process launcher.
 */

#pragma once

#include "infra/platform/IPlatformProcessRunner.h"

namespace infra::platform {

class Win32ProcessRunner final : public IPlatformProcessRunner {
public:
    ~Win32ProcessRunner() noexcept override = default;

    [[nodiscard]] bool supportsElevatedLaunch() const noexcept override;
    [[nodiscard]] bool startElevated(const QString& executablePath,
                                     const QStringList& arguments,
                                     QString* errorMessage) override;
};

} // namespace infra::platform
