/**
 * @file UpdateManager.cpp
 * @brief Implementation of UpdateManager.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "UpdateManager.h"
#include "../Config.h"
#include "PlatformUpdateInstallStrategy.h"
#include "infra/platform/PathResolver.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QStringList>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <spdlog/spdlog.h>

namespace core::update {

namespace {

struct ChecksumResult {
    bool success = false;
    QString error;
    QString expected;
    QString actual;
};

ChecksumResult computeSha256WithCancel(const QString& filePath,
                                       const QString& expectedSha,
                                       const QString& checksumsPath,
                                       const QString& targetFileName,
                                       std::shared_ptr<std::atomic_bool> cancelToken) {
    QString actualSha;
    QString resolvedExpectedSha = expectedSha.trimmed().toLower();

    // Resolve expected SHA from file if needed
    if (resolvedExpectedSha.isEmpty() && !checksumsPath.isEmpty()) {
        QFile file(checksumsPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QStringList lines = QString::fromUtf8(file.readAll()).split('\n');
            const QRegularExpression pattern(QStringLiteral("^\\s*([a-fA-F0-9]{64})\\s+\\*?(.+?)\\s*$"));
            for (const QString& line : lines) {
                const QRegularExpressionMatch match = pattern.match(line);
                if (match.hasMatch()) {
                    const QString fileName = QFileInfo(match.captured(2).trimmed()).fileName();
                    if (fileName.compare(targetFileName, Qt::CaseInsensitive) == 0) {
                        resolvedExpectedSha = match.captured(1).toLower();
                        break;
                    }
                }
            }
        }
    }

    if (resolvedExpectedSha.isEmpty()) {
        return {false, QObject::tr("Missing or invalid expected checksum"), {}, {}};
    }

    if (cancelToken && cancelToken->load(std::memory_order_relaxed)) {
        return {false, QObject::tr("Canceled"), {}, {}};
    }

    // Calculate actual SHA with block-by-block reading for responsive cancellation
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {false, QObject::tr("Failed to open file for checksum calculation"), {}, {}};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    constexpr qint64 kChunkSize = 64 * 1024;
    while (!file.atEnd()) {
        if (cancelToken && cancelToken->load(std::memory_order_relaxed)) {
            return {false, QObject::tr("Canceled"), {}, {}};
        }
        const QByteArray chunk = file.read(kChunkSize);
        if (chunk.isEmpty() && !file.atEnd()) {
            return {false, QObject::tr("Failed to read file chunk"), {}, {}};
        }
        hash.addData(chunk);
    }

    actualSha = hash.result().toHex();
    if (actualSha.isEmpty()) {
        return {false, QObject::tr("Failed to calculate file checksum"), {}, {}};
    }

    if (actualSha.compare(resolvedExpectedSha, Qt::CaseInsensitive) != 0) {
        return {false,
                QObject::tr("Checksum mismatch. Expected: %1, Actual: %2").arg(resolvedExpectedSha, actualSha),
                resolvedExpectedSha,
                actualSha};
    }

    return {true, {}, resolvedExpectedSha, actualSha};
}

} // anonymous namespace

UpdateManager::UpdateManager(QObject* parent,
                             std::unique_ptr<infra::platform::IPlatformProcessRunner> processRunner,
                             std::unique_ptr<PlatformUpdateInstallStrategy> installStrategy,
                             const infra::platform::PathResolver* pathResolver)
    : QObject(parent),
      networkManager_(new QNetworkAccessManager(this)),
      cancelToken_(std::make_shared<std::atomic_bool>(false)),
      processRunner_(std::move(processRunner)),
      installStrategy_(std::move(installStrategy)),
      pathResolver_(pathResolver) {
    if (!processRunner_) {
        processRunner_ = infra::platform::createDefaultPlatformProcessRunner();
    }
    if (!installStrategy_) {
        installStrategy_ = createPlatformUpdateInstallStrategy(pathResolver);
    }
}

UpdateManager::~UpdateManager() {
    cancelToken_->store(true);
    if (currentReply_) {
        currentReply_->disconnect(this);
        currentReply_->abort();
        currentReply_ = nullptr;
    }
}

UpdateInstallMode UpdateManager::installMode() const noexcept
{
    if (!installStrategy_) {
        return UpdateInstallMode::DownloadOnly;
    }
    return installStrategy_->installMode(processRunner_.get());
}

void UpdateManager::startUpdate(const QUrl& updateUrl, 
                                const QString& expectedSha, 
                                const QString& checksumsUrl,
                                const QString& latestVersion) {
    cancelUpdate();
    cancelToken_->store(false);
    pendingLatestVersion_ = latestVersion;

    // Staging area lives next to the executable (portable-only deployment,
    // PathResolver facade); a null resolver (unit tests) falls back to the
    // default resolver rooted at QCoreApplication::applicationDirPath().
    const QDir workingDir(updateStagingDir());
    if (!workingDir.exists() && !workingDir.mkpath(QStringLiteral("."))) {
        emit updateFailed(tr("Failed to create update directory"));
        return;
    }

    const QString updateFileName = QFileInfo(updateUrl.path()).fileName();
    if (updateFileName.isEmpty()) {
        emit updateFailed(tr("Invalid update URL"));
        return;
    }

    const QString updateFilePath = workingDir.filePath(updateFileName);

    SPDLOG_INFO("UpdateManager: Starting download from {}", updateUrl.toString().toStdString());

    downloadAsset(updateUrl, updateFilePath, [this, updateFilePath, expectedSha, checksumsUrl](bool success, const QString& error) {
        if (!success) {
            if (!cancelToken_->load()) {
                emit updateFailed(error);
            }
            return;
        }

        if (expectedSha.isEmpty() && !checksumsUrl.isEmpty()) {
            const QString checksumsPath = QFileInfo(updateFilePath).dir().filePath("sha256sums.txt");
            downloadAsset(QUrl(checksumsUrl), checksumsPath, [this, updateFilePath, checksumsPath](bool csSuccess, const QString& csError) {
                if (!csSuccess) {
                    emit updateFailed(csError);
                    return;
                }
                processDownloadedUpdate(updateFilePath, QString(), checksumsPath);
            });
            return;
        }

        SPDLOG_INFO("UpdateManager: Download finished, starting verification...");
        processDownloadedUpdate(updateFilePath, expectedSha, QString());
    });
}

void UpdateManager::cancelUpdate() {
    cancelToken_->store(true);
    if (currentReply_) {
        currentReply_->abort();
    }
}

void UpdateManager::downloadAsset(const QUrl& url, const QString& filePath, std::function<void(bool, const QString&)> onFinished) {
    if (currentReply_) {
        currentReply_->abort();
    }

    outputFile_ = std::make_unique<QFile>(filePath);
    if (!outputFile_->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        onFinished(false, tr("Failed to open local file for writing"));
        return;
    }

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    currentReply_ = networkManager_->get(request);

    connect(currentReply_, &QNetworkReply::readyRead, this, [this]() {
        if (currentReply_ && outputFile_) {
            outputFile_->write(currentReply_->readAll());
        }
    });

    connect(currentReply_, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64 total) {
        if (total > 0) {
            emit downloadProgress(static_cast<int>((received * 100) / total));
        }
    });

    connect(currentReply_, &QNetworkReply::finished, this, [this, onFinished]() {
        onDownloadFinished(onFinished);
    });
}

void UpdateManager::onDownloadFinished(std::function<void(bool, const QString&)> onFinished) {
    if (!currentReply_) return;
    const bool canceled = cancelToken_->load() || (currentReply_->error() == QNetworkReply::OperationCanceledError);
    if (currentReply_->error() != QNetworkReply::NoError) {
        QString error = currentReply_->errorString();
        currentReply_->deleteLater();
        outputFile_->close();
        if (canceled) {
            emit updateCanceled();
        } else {
            onFinished(false, error);
        }
        return;
    }
    outputFile_->write(currentReply_->readAll());
    outputFile_->close();
    currentReply_->deleteLater();
    onFinished(true, {});
}

void UpdateManager::processDownloadedUpdate(const QString& updateFilePath, const QString& expectedSha, const QString& checksumsPath) {
    const QString fileName = QFileInfo(updateFilePath).fileName();
    auto* watcher = new QFutureWatcher<ChecksumResult>(this);
    const QPointer<UpdateManager> weakThis(this);

    connect(watcher, &QFutureWatcher<ChecksumResult>::finished, this, [this, watcher, weakThis, updateFilePath]() {
        const ChecksumResult res = watcher->result();
        watcher->deleteLater();
        if (!weakThis) {
            return;
        }

        if (cancelToken_->load(std::memory_order_relaxed)) {
            emit updateCanceled();
            return;
        }

        if (!res.success) {
            SPDLOG_ERROR("UpdateManager: Update verification failed: {}", res.error.toStdString());
            emit updateFailed(res.error);
            return;
        }

        SPDLOG_INFO("UpdateManager: Verification successful. Expected: {}, Actual: {}",
                     res.expected.toStdString(), res.actual.toStdString());

        if (!installStrategy_) {
            emit updateFailed(tr("No update install strategy available"));
            return;
        }

        PreparedUpdateContext context;
        context.updateFilePath = updateFilePath;
        context.latestVersion = pendingLatestVersion_;
        context.expectedSha256 = res.expected;
        context.applicationFilePath = QCoreApplication::applicationFilePath();

        QString installArtifactPath;
        QString errorMessage;
        if (!installStrategy_->createInstallArtifact(context, installArtifactPath, errorMessage)) {
            emit updateFailed(errorMessage);
            return;
        }

        SPDLOG_INFO("UpdateManager: Update install artifact created at {}",
                     installArtifactPath.toStdString());
        emit updateReadyToInstall(installArtifactPath);
    });

    watcher->setFuture(QtConcurrent::run([updateFilePath, expectedSha, checksumsPath, fileName, cancelToken = cancelToken_]() {
        return computeSha256WithCancel(updateFilePath, expectedSha, checksumsPath, fileName, cancelToken);
    }));
}

bool UpdateManager::launchInstaller(const QString& installArtifactPath, const QString& langCode, QString& errorMessage) {
    if (!installStrategy_) {
        errorMessage = tr("No update install strategy available");
        return false;
    }

    const bool launched = installStrategy_->launchInstallArtifact(
        installArtifactPath, langCode, processRunner_.get(), errorMessage);
    if (launched) {
        SPDLOG_INFO("UpdateManager: Installer launched successfully. Artifact: {}",
                     installArtifactPath.toStdString());
    }
    return launched;
}

void UpdateManager::cleanupUpdateArtifacts() {
    SPDLOG_INFO("UpdateManager: Cleaning up temporary update artifacts...");

    // Static context: no member resolver available; a default-constructed
    // resolver roots at QCoreApplication::applicationDirPath() — the same
    // staging location UpdateManager instances resolve to in production.
    const QString stagingDir = infra::platform::PathResolver{}.resolveUpdateStagingDir();
    if (!stagingDir.isEmpty()) {
        QDir(stagingDir).removeRecursively();
    }
}

QString UpdateManager::updateStagingDir() const
{
    if (pathResolver_ != nullptr) {
        return pathResolver_->resolveUpdateStagingDir();
    }
    return infra::platform::PathResolver{}.resolveUpdateStagingDir();
}

} // namespace core::update
