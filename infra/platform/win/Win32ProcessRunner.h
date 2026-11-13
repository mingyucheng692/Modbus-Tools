// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "infra/platform/IPlatformProcessRunner.h"

namespace infra::platform {

class Win32ProcessRunner final : public IPlatformProcessRunner {
public:
    ~Win32ProcessRunner() noexcept override = default;

    /// Quotes a single command-line argument using Microsoft CommandLineToArgvW rules.
    [[nodiscard]] static QString quoteProcessArgument(const QString& argument);

    /// Joins command-line arguments after quoting each element with quoteProcessArgument().
    [[nodiscard]] static QString buildProcessArguments(const QStringList& arguments);

    [[nodiscard]] bool supportsElevatedLaunch() const noexcept override;
    [[nodiscard]] bool startElevated(const QString& executablePath,
                                     const QStringList& arguments,
                                     QString* errorMessage) override;
    [[nodiscard]] bool startNonElevated(const QString& executablePath,
                                        const QStringList& arguments,
                                        QString* errorMessage) override;
};

} // namespace infra::platform
