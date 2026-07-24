/**
 * @file PathResolver.h
 * @brief Declares the writable path resolver for installed and portable deployments.
 *
 * Per AGENTS.md §21.7: portable mode is opt-in only (`.portable` marker file
 * next to the executable OR `--portable` CLI argument). Installed mode is the
 * default and prefers QStandardPaths locations. Direct inference from
 * "directory happens to be writable" is forbidden.
 *
 * Free functions appDataLocation(), appConfigLocation(), tempLocation() provide
 * the default QStandardPaths-backed resolution. PathResolver uses these by
 * default; tests may inject std::function overrides.
 */

#pragma once

#include <QString>
#include <QStringList>
#include <functional>

namespace infra::platform {

/// Free functions directly wrapping QStandardPaths::writableLocation.
[[nodiscard]] QString appDataLocation();
[[nodiscard]] QString appConfigLocation();
[[nodiscard]] QString tempLocation();

class PathResolver final {
public:
    /// Default constructor: uses the free functions above (backed by
    /// QStandardPaths) and detects portable mode from `.portable` marker file
    /// or `--portable` CLI argument.
    PathResolver();

    /// Test/injection constructor. Each std::function may be default-constructed
    /// (equiv. to not provided) — the resolver will fall back to the free
    /// function. `arguments` is typically QCoreApplication::arguments();
    /// an empty list means "no CLI arguments" (installed mode unless marker file exists).
    PathResolver(std::function<QString()> appDataLocationFn,
                 std::function<QString()> appConfigLocationFn,
                 std::function<QString()> tempLocationFn,
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

    std::function<QString()> appDataLocationFn_;
    std::function<QString()> appConfigLocationFn_;
    std::function<QString()> tempLocationFn_;
    QString applicationDirPath_;
    QString applicationName_;
    bool portableMode_ = false;
};

} // namespace infra::platform