/**
 * @file PathResolver.h
 * @brief Declares writable path resolution for portable and standard installs.
 *
 * Portable mode (`.portable` marker file or `--portable` CLI argument) uses
 * the application directory for config/logs. Standard mode uses
 * QStandardPaths-backed locations, with scoped temp as fallback.
 */

#pragma once

#include <functional>
#include <utility>
#include <QString>
#include <QStringList>

namespace infra::platform {

class PathResolver final {
public:
    using StandardPathProvider = std::function<QString()>;

    /// Default constructor: uses QCoreApplication for applicationDirPath,
    /// arguments, applicationName, and QStandardPaths-backed standard dirs.
    PathResolver();

    /// Constructor for tests and explicit injection.
    PathResolver(QString applicationDirPath,
                 QStringList arguments,
                 QString applicationName);

    /// Constructor for tests that need deterministic standard locations.
    PathResolver(StandardPathProvider appDataDirProvider,
                 StandardPathProvider appConfigDirProvider,
                 StandardPathProvider tempDirProvider,
                 QString applicationDirPath,
                 QStringList arguments,
                 QString applicationName);

    /// Returns true when portable mode was explicitly opted-in (marker file or --portable).
    [[nodiscard]] bool isPortableMode() const noexcept { return portableMode_; }

    /// Application directory captured at construction. Exposed for consumers
    /// that must resolve sibling binaries (e.g. the bundled updater) instead of
    /// calling QCoreApplication::applicationDirPath() directly (Task 2.1).
    [[nodiscard]] const QString& applicationDirPath() const noexcept { return applicationDirPath_; }

    [[nodiscard]] QString resolveLogDir() const;
    [[nodiscard]] QString resolveConfigDir() const;
    [[nodiscard]] QString resolveTempDir() const;

private:
    void detectPortableMode(const QStringList& arguments);

    [[nodiscard]] QString resolveScopedTempDir() const;
    [[nodiscard]] QString resolveWritableDir(const QString& purpose,
                                            const QString& preferredDir,
                                            const QString& fallbackDir) const;
    [[nodiscard]] bool isWritableDirectory(const QString& directoryPath) const;

    [[nodiscard]] static QString currentAppDataDirPath();
    [[nodiscard]] static QString currentAppConfigDirPath();
    [[nodiscard]] static QString currentApplicationDirPath();
    [[nodiscard]] static QString currentApplicationName();
    [[nodiscard]] static QStringList currentApplicationArguments();
    [[nodiscard]] static QString currentTempRootDirPath();

    QString applicationDirPath_;
    QString applicationName_;
    StandardPathProvider appDataDirProvider_;
    StandardPathProvider appConfigDirProvider_;
    StandardPathProvider tempDirProvider_;
    bool portableMode_ = false;
};

} // namespace infra::platform
