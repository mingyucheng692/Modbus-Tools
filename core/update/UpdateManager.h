// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QPointer>
#include "infra/platform/IPlatformProcessRunner.h"
#include "PlatformReleaseAssetStrategy.h"
#include "PlatformUpdateInstallStrategy.h"
#include <memory>
#include <atomic>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QFile;

namespace infra::platform {
class PathResolver;
}

namespace core::update {

enum class UpdateInstallMode {
    AutomaticInstaller,
    DownloadOnly
};

// Role: Result container for update artifact download and checksum verification.
// Thread-safety: Value type; thread-safe for independent instances.
// Ownership: Value object passed across async callbacks or signals.
struct UpdatePreparationResult {
    bool success = false;
    QString errorMessage;
    QString taskFilePath;
    QString expectedSha;
    QString actualSha;
};

// Role: Bundled update metadata (version tags, asset download URLs, checksums, install guidance).
// Thread-safety: Value type; thread-safe for independent instances.
// Ownership: Value object passed by const reference across UI and update layers.
struct UpdateInfo {
    QString currentVersion;
    QString latestVersion;
    QString updateOnlyUrl;
    QString updateOnlySha256;
    QString checksumsUrl;
    QString fullPackageUrl;
    QString releaseUrl;
    /// Published SHA256 of the full package asset; empty means the release
    /// carries no digest — downstream UI must surface that explicitly.
    QString fullPackageSha256;
    /// How the user should apply this update; sourced from the platform
    /// layout table (single platform-delta point, no UI-level OS branches).
    UpdateGuidance guidance = UpdateGuidance::OpenDownloadPage;
};

// Role: Manages update download, checksum verification, and updater launch.
// Thread-safety: Single-threaded; bound to the GUI thread.
// Ownership: Owned by application composition root via QObject parent-child hierarchy.
class UpdateManager : public QObject {
    Q_OBJECT

public:
    explicit UpdateManager(QObject* parent = nullptr,
                           std::unique_ptr<infra::platform::IPlatformProcessRunner> processRunner = {},
                           std::unique_ptr<PlatformUpdateInstallStrategy> installStrategy = {},
                           const infra::platform::PathResolver* pathResolver = nullptr);
    ~UpdateManager() noexcept override;

    void startUpdate(const QUrl& updateUrl, 
                     const QString& expectedSha, 
                     const QString& checksumsUrl,
                     const QString& latestVersion);

    void cancelUpdate();

    /// Virtual test seam: unit tests substitute FakeUpdateManager to
    /// force DownloadOnly behavior per platform.
    [[nodiscard]] virtual UpdateInstallMode installMode() const noexcept;

    [[nodiscard]] bool launchInstaller(const QString& installArtifactPath,
                                       const QString& langCode,
                                       QString& errorMessage);

    static void cleanupUpdateArtifacts();

signals:
    void downloadProgress(int progress);

    void updateReadyToInstall(const QString& taskFilePath);

    void updateFailed(const QString& reason);

    void updateCanceled();

private:
    void processDownloadedUpdate(const QString& updateFilePath,
                                 const QString& expectedSha,
                                 const QString& checksumsPath);

    void downloadAsset(const QUrl& url,
                       const QString& filePath,
                       std::function<void(bool, const QString&)> onFinished);

    void onDownloadFinished(std::function<void(bool, const QString&)> onFinished);

    /// Update staging dir via the injected resolver; falls back to a
    /// default resolver rooted at applicationDirPath() when none was
    /// supplied (unit tests).
    [[nodiscard]] QString updateStagingDir() const;

    QString pendingLatestVersion_;
    std::shared_ptr<std::atomic_bool> cancelToken_;

    // Low-level download state
    QNetworkAccessManager* networkManager_ = nullptr;
    QPointer<QNetworkReply> currentReply_;
    std::unique_ptr<QFile> outputFile_;
    std::unique_ptr<infra::platform::IPlatformProcessRunner> processRunner_;
    std::unique_ptr<PlatformUpdateInstallStrategy> installStrategy_;
    const infra::platform::PathResolver* pathResolver_ = nullptr;
};

} // namespace core::update
