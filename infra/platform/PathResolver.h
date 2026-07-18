/**
 * @file PathResolver.h
 * @brief Declares the writable path resolver for installed and portable deployments.
 *
 * Per AGENTS.md §21.7: portable mode is opt-in only (`.portable` marker file
 * next to the executable OR `--portable` CLI argument). Installed mode is the
 * default and prefers QStandardPaths locations. Direct inference from
 * "directory happens to be writable" is forbidden.
 */

#pragma once

#include "infra/platform/IPlatformPaths.h"

#include <QString>
#include <QStringList>
#include <memory>

namespace infra::platform {

class PathResolver final {
public:
    /// Default constructor: uses QtStandardPlatformPaths and detects portable
    /// mode from `.portable` marker file or `--portable` CLI argument.
    PathResolver();

    /// Test/injection constructor. `arguments` is typically QCoreApplication::arguments();
    /// an empty list means "no CLI arguments" (installed mode unless marker file exists).
    PathResolver(std::shared_ptr<const IPlatformPaths> platformPaths,
                 QString applicationDirPath,
                 QStringList arguments,
                 QString applicationName);

    /// Returns true when portable mode was explicitly opted-in (marker file or --portable).
    [[nodiscard]] bool isPortableMode() const noexcept { return portableMode_; }

    [[nodiscard]] QString resolveLogDir() const;
    [[nodiscard]] QString resolveConfigDir() const;
    [[nodiscard]] QString resolveTempDir() const;

private:
    void detectPortableMode(const QStringList& arguments);

    [[nodiscard]] QString resolveScopedTempDir() const;
    [[nodiscard]] QString resolveWritableDir(const QString& purpose,
                                            const QString& preferredDir,
                                            const QString& secondaryDir,
                                            const QString& fallbackDir) const;
    [[nodiscard]] bool isWritableDirectory(const QString& directoryPath) const;

    [[nodiscard]] static QString currentApplicationDirPath();
    [[nodiscard]] static QString currentApplicationName();
    [[nodiscard]] static QStringList currentApplicationArguments();

    std::shared_ptr<const IPlatformPaths> platformPaths_;
    QString applicationDirPath_;
    QString applicationName_;
    bool portableMode_ = false;
};

} // namespace infra::platform
