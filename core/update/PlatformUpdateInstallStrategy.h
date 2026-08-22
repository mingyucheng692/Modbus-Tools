/**
 * @file PlatformUpdateInstallStrategy.h
 * @brief Declares platform-specific OTA install strategy helpers.
 */

#pragma once

#include <QString>
#include <memory>

namespace infra::platform {
class IPlatformProcessRunner;
class PathResolver;
}

namespace core::update {

enum class UpdateInstallMode;

struct PreparedUpdateContext {
    QString updateFilePath;
    QString latestVersion;
    QString expectedSha256;
    QString applicationFilePath;
};

class PlatformUpdateInstallStrategy {
public:
    virtual ~PlatformUpdateInstallStrategy() noexcept = default;

    [[nodiscard]] virtual UpdateInstallMode installMode(
        const infra::platform::IPlatformProcessRunner* processRunner) const noexcept = 0;
    [[nodiscard]] virtual bool createInstallArtifact(const PreparedUpdateContext& context,
                                                     QString& installArtifactPath,
                                                     QString& errorMessage) const = 0;
    [[nodiscard]] virtual bool launchInstallArtifact(const QString& installArtifactPath,
                                                     const QString& langCode,
                                                     infra::platform::IPlatformProcessRunner* processRunner,
                                                     QString& errorMessage) const = 0;
};

/// Creates the install strategy for the compiled platform. The optional
/// PathResolver supplies the application directory used to locate the bundled
/// updater binary; when null the strategy falls back to
/// QCoreApplication::applicationDirPath().
[[nodiscard]] std::unique_ptr<PlatformUpdateInstallStrategy> createPlatformUpdateInstallStrategy(
    const infra::platform::PathResolver* pathResolver = nullptr);

} // namespace core::update
