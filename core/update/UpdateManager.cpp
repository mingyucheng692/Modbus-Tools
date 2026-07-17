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
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QThread>
#include <QRegularExpression>
#include <QStringList>
#include <spdlog/spdlog.h>

#if defined(Q_OS_WIN)
#include "infra/platform/win/Win32ProcessRunner.h"
#endif

namespace {
[[nodiscard]] static std::unique_ptr<infra::platform::IPlatformProcessRunner> createPlatformProcessRunner()
{
#if defined(Q_OS_WIN)
    return std::make_unique<Win32ProcessRunner>();
#else
    return nullptr; // Not supported on this platform
#endif
}
} // namespace

namespace core::update {

/**
 * @brief Worker for CPU-intensive checksum calculations.
 */
class ChecksumWorker : public QObject {
    Q_OBJECT
public:
    explicit ChecksumWorker(QObject* parent = nullptr) : QObject(parent) {}

    void calculateSha256(const QString& filePath, const QString& expectedSha, const QString& checksumsPath, const QString& targetFileName) {
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
            emit finished(false, tr("Missing or invalid expected checksum"), {}, {});
            return;
        }

        // Calculate actual SHA
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QCryptographicHash hash(QCryptographicHash::Sha256);
            if (hash.addData(&file)) {
                actualSha = hash.result().toHex();
            }
        }

        if (actualSha.isEmpty()) {
            emit finished(false, tr("Failed to calculate file checksum"), {}, {});
            return;
        }

        if (actualSha.compare(resolvedExpectedSha, Qt::CaseInsensitive) != 0) {
            emit finished(false, tr("Checksum mismatch. Expected: %1, Actual: %2").arg(resolvedExpectedSha, actualSha), resolvedExpectedSha, actualSha);
        } else {
            emit finished(true, {}, resolvedExpectedSha, actualSha);
        }
    }

signals:
    void finished(bool success, const QString& error, const QString& expected, const QString& actual);
};

UpdateManager::UpdateManager(QObject* parent,
                             std::unique_ptr<infra::platform::IPlatformProcessRunner> processRunner,
                             std::unique_ptr<PlatformUpdateInstallStrategy> installStrategy)
    : QObject(parent),
      networkManager_(new QNetworkAccessManager(this)),
      cancelToken_(std::make_shared<std::atomic_bool>(false)),
      processRunner_(std::move(processRunner)),
      installStrategy_(std::move(installStrategy)) {
    if (!processRunner_) {
        processRunner_ = createPlatformProcessRunner();
    }
    if (!installStrategy_) {
        installStrategy_ = createPlatformUpdateInstallStrategy();
    }
}

UpdateManager::~UpdateManager() {
    cancelUpdate();
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

    const QString tempRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (tempRoot.isEmpty()) {
        emit updateFailed(tr("Failed to access system temporary directory"));
        return;
    }

    const QDir workingDir(QStringLiteral("%1/ModbusToolsUpdate").arg(tempRoot));
    if (!workingDir.exists() && !workingDir.mkpath(".")) {
        emit updateFailed(tr("Failed to create update directory"));
        return;
    }

    const QString updateFileName = QFileInfo(updateUrl.path()).fileName();
    if (updateFileName.isEmpty()) {
        emit updateFailed(tr("Invalid update URL"));
        return;
    }

    const QString updateFilePath = workingDir.filePath(updateFileName);

    spdlog::info("UpdateManager: Starting download from {}", updateUrl.toString().toStdString());

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

        spdlog::info("UpdateManager: Download finished, starting verification...");
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
    });
}

void UpdateManager::processDownloadedUpdate(const QString& updateFilePath, const QString& expectedSha, const QString& checksumsPath) {
    auto* thread = new QThread(this);
    auto* worker = new ChecksumWorker();
    worker->moveToThread(thread);

    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);

    connect(worker, &ChecksumWorker::finished, this, [this, updateFilePath, thread](bool success, const QString& error, const QString& expected, const QString& actual) {
        if (!success) {
            spdlog::error("UpdateManager: Update verification failed: {}", error.toStdString());
            emit updateFailed(error);
            thread->quit();
            return;
        }

        spdlog::info("UpdateManager: Verification successful. Expected: {}, Actual: {}", expected.toStdString(), actual.toStdString());

        if (!installStrategy_) {
            emit updateFailed(tr("No update install strategy available"));
            thread->quit();
            return;
        }

        PreparedUpdateContext context;
        context.updateFilePath = updateFilePath;
        context.latestVersion = pendingLatestVersion_;
        context.expectedSha256 = expected;
        context.applicationFilePath = QCoreApplication::applicationFilePath();

        QString installArtifactPath;
        QString errorMessage;
        if (!installStrategy_->createInstallArtifact(context, installArtifactPath, errorMessage)) {
            emit updateFailed(errorMessage);
            thread->quit();
            return;
        }

        spdlog::info("UpdateManager: Update install artifact created at {}",
                     installArtifactPath.toStdString());
        emit updateReadyToInstall(installArtifactPath);
        
        thread->quit();
    });

    thread->start();
    const QString fileName = QFileInfo(updateFilePath).fileName();
    QMetaObject::invokeMethod(worker, [worker, updateFilePath, expectedSha, checksumsPath, fileName]() {
        worker->calculateSha256(updateFilePath, expectedSha, checksumsPath, fileName);
    }, Qt::QueuedConnection);
}

bool UpdateManager::launchInstaller(const QString& installArtifactPath, const QString& langCode, QString& errorMessage) {
    if (!installStrategy_) {
        errorMessage = tr("No update install strategy available");
        return false;
    }

    const bool launched = installStrategy_->launchInstallArtifact(
        installArtifactPath, langCode, processRunner_.get(), errorMessage);
    if (launched) {
        spdlog::info("UpdateManager: Installer launched successfully. Artifact: {}",
                     installArtifactPath.toStdString());
    }
    return launched;
}

void UpdateManager::cleanupUpdateArtifacts() {
    spdlog::info("UpdateManager: Cleaning up temporary update artifacts...");

    const QString tempRoot = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    if (!tempRoot.isEmpty()) {
        QDir(QStringLiteral("%1/ModbusToolsUpdate").arg(tempRoot)).removeRecursively();
    }
}

} // namespace core::update

#include "UpdateManager.moc"
