/**
 * @file PathResolver.h
 * @brief Declares the writable path resolver for portable deployments.
 *
 * Portable-first design: always prefers the application directory with
 * QDir::tempPath() as fallback. No QStandardPaths involvement.
 *
 * Portable mode (`.portable` marker file or `--portable` CLI argument) is
 * detected for informational purposes (isPortableMode()) but does not change
 * the resolution logic — all modes use exe-dir-first.
 */

#pragma once

#include <QString>
#include <QStringList>

namespace infra::platform {

class PathResolver final {
public:
    /// Default constructor: uses QCoreApplication for applicationDirPath,
    /// arguments, and applicationName.
    PathResolver();

    /// Constructor for tests and explicit injection.
    PathResolver(QString applicationDirPath,
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
                                            const QString& fallbackDir) const;
    [[nodiscard]] bool isWritableDirectory(const QString& directoryPath) const;

    [[nodiscard]] static QString currentApplicationDirPath();
    [[nodiscard]] static QString currentApplicationName();
    [[nodiscard]] static QStringList currentApplicationArguments();

    QString applicationDirPath_;
    QString applicationName_;
    bool portableMode_ = false;
};

} // namespace infra::platform