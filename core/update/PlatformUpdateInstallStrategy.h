/**
 * @file PlatformUpdateInstallStrategy.h
 * @brief Declares platform-specific OTA install strategy helpers.
 */

#pragma once

#include <QJsonObject>
#include <QString>
#include <memory>

namespace infra::platform {
class IPlatformProcessRunner;
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

protected:
    [[nodiscard]] static QJsonObject buildWindowsTaskDocument(const PreparedUpdateContext& context);
};

[[nodiscard]] std::unique_ptr<PlatformUpdateInstallStrategy> createPlatformUpdateInstallStrategy();

} // namespace core::update
