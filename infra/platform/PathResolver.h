/**
 * @file PathResolver.h
 * @brief Declares the writable path resolver for installed and portable deployments.
 */

#pragma once

#include "infra/platform/IPlatformPaths.h"

#include <QString>
#include <QStringList>
#include <memory>

namespace infra::platform {

class PathResolver final {
public:
    PathResolver();
    PathResolver(std::shared_ptr<const IPlatformPaths> platformPaths,
                 QString applicationDirPath,
                 QStringList arguments,
                 QString applicationName);

    [[nodiscard]] static PathResolver& instance();

    [[nodiscard]] QString resolveLogDir() const;
    [[nodiscard]] QString resolveConfigDir() const;
    [[nodiscard]] QString resolveTempDir() const;
    [[nodiscard]] bool isPortableMode() const;

private:
    [[nodiscard]] QString resolveScopedTempDir() const;
    [[nodiscard]] QString resolveWritableDir(const QString& purpose,
                                            const QString& preferredDir,
                                            const QString& secondaryDir,
                                            const QString& fallbackDir) const;
    [[nodiscard]] bool isWritableDirectory(const QString& directoryPath) const;
    [[nodiscard]] bool hasPortableMarker() const;
    [[nodiscard]] QString portableMarkerPath() const;

    [[nodiscard]] static QString currentApplicationDirPath();
    [[nodiscard]] static QStringList currentArguments();
    [[nodiscard]] static QString currentApplicationName();

    std::shared_ptr<const IPlatformPaths> platformPaths_;
    QString applicationDirPath_;
    QStringList arguments_;
    QString applicationName_;
};

} // namespace infra::platform
