/**
 * @file PathResolver.h
 * @brief Declares writable path resolution for the portable-only deployment.
 *
 * The application is a portable tool: every writable artifact (config, logs,
 * update staging) lives next to the executable, typically on the Desktop or
 * a USB drive. There is deliberately NO QStandardPaths/AppData branch and NO
 * fallback chain — re-introducing one would silently scatter user data into
 * %LOCALAPPDATA% again.
 *
 * Writability is not probed here. If the directory cannot be written, the
 * log initialization surfaces a one-time dialog (see main.cpp) suggesting
 * relocation to a writable folder, and the app keeps running in degraded
 * mode.
 */

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
    /// calling QCoreApplication::applicationDirPath() directly (Task 2.1).
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
