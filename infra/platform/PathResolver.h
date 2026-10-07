// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>

namespace infra::platform {

class PathResolver final {
public:
    /// Default constructor: captures QCoreApplication::applicationDirPath().
    PathResolver();

    /// Constructor for tests and explicit injection.
    explicit PathResolver(QString applicationDirPath);

    /// Application directory captured at construction. Exposed for consumers
    /// that must resolve sibling binaries (e.g. the bundled updater) instead of
    /// calling QCoreApplication::applicationDirPath() directly.
    [[nodiscard]] const QString& applicationDirPath() const noexcept { return applicationDirPath_; }

    /// <exeDir>/logs/ — best-effort mkpath; callers own writability handling.
    [[nodiscard]] QString resolveLogDir() const;

    /// <exeDir>/ — config.ini lives directly next to the executable.
    [[nodiscard]] QString resolveConfigDir() const;

    /// <exeDir>/update/ — transient update download staging, removed after install.
    [[nodiscard]] QString resolveUpdateStagingDir() const;

private:
    QString applicationDirPath_;
};

} // namespace infra::platform
