#pragma once

#include <QObject>
#include <QString>
#include <QUrl>
#include <QPointer>
#include <memory>
#include <atomic>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QFile;

namespace core::update {

/**
 * @brief Result of the update preparation process (download + verify).
 */
struct UpdatePreparationResult {
    bool success = false;
    QString errorMessage;
    QString taskFilePath;
    QString expectedSha;
    QString actualSha;
};

/**
 * @brief Manages the application update process, including downloading, 
 *        checksum verification, and launching the external updater.
 */
class UpdateManager : public QObject {
    Q_OBJECT

public:
    explicit UpdateManager(QObject* parent = nullptr);
    ~UpdateManager() override;

    /**
     * @brief Starts the silent update process.
     * @param updateUrl URL of the update package.
     * @param expectedSha Expected SHA256 checksum (optional).
     * @param checksumsUrl URL of the checksums file (optional).
     * @param latestVersion Version string of the update.
     */
    void startUpdate(const QUrl& updateUrl, 
                     const QString& expectedSha, 
                     const QString& checksumsUrl,
                     const QString& latestVersion);

    /**
     * @brief Cancels any ongoing update process.
     */
    void cancelUpdate();

    /**
     * @brief Launches the external updater tool.
     * @param taskFilePath Path to the update task JSON file.
     * @param errorMessage Output parameter for error messages.
     * @return true if launched successfully.
     */
    static bool launchUpdater(const QString& taskFilePath, QString& errorMessage);

    /**
     * @brief Cleans up temporary update artifacts.
     */
    static void cleanupUpdateArtifacts();

signals:
    /**
     * @brief Emitted when download progress updates.
     * @param progress Percentage (0-100).
     */
    void downloadProgress(int progress);

    /**
     * @brief Emitted when the update package is ready to be applied.
     */
    void updateReadyToInstall();

    /**
     * @brief Emitted when the update process fails.
     * @param reason Description of the failure.
     */
    void updateFailed(const QString& reason);

    /**
     * @brief Emitted when the update process is canceled.
     */
    void updateCanceled();

private:
    void processDownloadedUpdate(const QString& updateFilePath,
                                 const QString& expectedSha,
                                 const QString& checksumsPath);

    void downloadAsset(const QUrl& url, 
                       const QString& filePath, 
                       std::function<void(bool, const QString&)> onFinished);

    QString pendingLatestVersion_;
    std::shared_ptr<std::atomic_bool> cancelToken_;
    
    // Low-level download state
    QNetworkAccessManager* networkManager_ = nullptr;
    QPointer<QNetworkReply> currentReply_;
    std::unique_ptr<QFile> outputFile_;
};

} // namespace core::update
