/**
 * @file PlatformUpdateInstallStrategy.cpp
 * @brief Implements platform-specific OTA install strategy helpers.
 */

#include "PlatformUpdateInstallStrategy.h"

#include "UpdateManager.h"
#include "infra/platform/IPlatformProcessRunner.h"
#include "infra/platform/PlatformInfo.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <spdlog/spdlog.h>

namespace {

#ifndef MODBUS_TOOLS_PLATFORM
#define MODBUS_TOOLS_PLATFORM "windows-x86_64"
#endif

QString currentPackagePlatform()
{
    return QStringLiteral(MODBUS_TOOLS_PLATFORM).toLower();
}

QString bundledUpdaterPath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("updater.exe"));
}

/// Computes SHA256 of a file. Returns empty string on failure.
QString computeFileSha256(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return {};
    }
    return QString::fromLatin1(hash.result().toHex());
}

/// Verifies the updater.exe binary integrity before launch.
/// Returns true if the check passes or is skipped (no expected hash configured).
bool verifyUpdaterIntegrity(const QString& updaterPath, const QString& expectedSha256, QString& errorDetail)
{
    if (!QFileInfo::exists(updaterPath)) {
        errorDetail = QStringLiteral("Updater binary not found at %1").arg(updaterPath);
        return false;
    }

    if (expectedSha256.isEmpty()) {
        // No expected hash configured — skip verification (development builds).
        return true;
    }

    const QString actualSha = computeFileSha256(updaterPath);
    if (actualSha.isEmpty()) {
        errorDetail = QStringLiteral("Failed to compute updater SHA256");
        return false;
    }

    if (actualSha.compare(expectedSha256.trimmed(), Qt::CaseInsensitive) != 0) {
        errorDetail = QStringLiteral("Updater integrity check failed. Expected: %1, Actual: %2")
                          .arg(expectedSha256.trimmed(), actualSha);
        return false;
    }

    return true;
}

class WindowsUpdateInstallStrategy final : public core::update::PlatformUpdateInstallStrategy {
public:
    [[nodiscard]] core::update::UpdateInstallMode installMode(
        const infra::platform::IPlatformProcessRunner* processRunner) const noexcept override
    {
        return (processRunner != nullptr && processRunner->supportsElevatedLaunch())
            ? core::update::UpdateInstallMode::AutomaticInstaller
            : core::update::UpdateInstallMode::DownloadOnly;
    }

    [[nodiscard]] bool createInstallArtifact(const core::update::PreparedUpdateContext& context,
                                             QString& installArtifactPath,
                                             QString& errorMessage) const override
    {
        const QString taskFilePath = QFileInfo(context.updateFilePath).dir().filePath(
            QStringLiteral("update_task.json"));
        QFile taskFile(taskFilePath);
        if (!taskFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            errorMessage = QCoreApplication::translate("core::update::UpdateManager",
                                                       "Failed to create update task file");
            return false;
        }

        taskFile.write(QJsonDocument(buildWindowsTaskDocument(context)).toJson());
        taskFile.close();
        installArtifactPath = taskFilePath;
        return true;
    }

    [[nodiscard]] bool launchInstallArtifact(const QString& installArtifactPath,
                                             const QString& langCode,
                                             infra::platform::IPlatformProcessRunner* processRunner,
                                             QString& errorMessage) const override
    {
        if (processRunner == nullptr || !processRunner->supportsElevatedLaunch()) {
            errorMessage = QCoreApplication::translate("core::update::UpdateManager",
                                                       "Automatic update is not supported on Windows");
            return false;
        }

        const QString updaterPath = bundledUpdaterPath();
        if (!QFileInfo::exists(updaterPath)) {
            errorMessage = QCoreApplication::translate("core::update::UpdateManager",
                                                       "Updater not found");
            return false;
        }

        // Verify updater.exe integrity before launching.
        {
            QString detail;
            if (!verifyUpdaterIntegrity(updaterPath, QStringLiteral(MODBUS_TOOLS_UPDATER_SHA256), detail)) {
                errorMessage = QCoreApplication::translate("core::update::UpdateManager",
                                                           "Updater integrity check failed: %1").arg(detail);
                return false;
            }
        }

        // Read task.json to extract parameters, then pass them via CLI
        // (not via --task file) to eliminate TOCTOU surface.
        QFile taskFile(installArtifactPath);
        if (!taskFile.open(QIODevice::ReadOnly)) {
            errorMessage = QCoreApplication::translate("core::update::UpdateManager",
                                                       "Failed to read update task file");
            return false;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(taskFile.readAll());
        taskFile.close();
        if (!doc.isObject()) {
            errorMessage = QCoreApplication::translate("core::update::UpdateManager",
                                                       "Invalid update task file");
            return false;
        }

        const QJsonObject root = doc.object();
        const QString targetExe = root.value(QStringLiteral("targetExePath")).toString();
        const QString newExe = root.value(QStringLiteral("newExePath")).toString();
        const QString backupExe = root.value(QStringLiteral("backupExePath")).toString();
        const QString expectedSha256 = root.value(QStringLiteral("expectedSha256")).toString();
        const QString expectedVersion = root.value(QStringLiteral("expectedVersion")).toString();
        const qint64 launcherPid = static_cast<qint64>(QCoreApplication::applicationPid());

        if (targetExe.isEmpty() || newExe.isEmpty() || expectedSha256.isEmpty()) {
            errorMessage = QCoreApplication::translate("core::update::UpdateManager",
                                                       "Incomplete update task parameters");
            return false;
        }

        const QStringList arguments{
            QStringLiteral("--target-exe"),
            QDir::toNativeSeparators(targetExe),
            QStringLiteral("--new-exe"),
            QDir::toNativeSeparators(newExe),
            QStringLiteral("--backup-exe"),
            QDir::toNativeSeparators(backupExe),
            QStringLiteral("--expected-sha256"),
            expectedSha256,
            QStringLiteral("--expected-version"),
            expectedVersion,
            QStringLiteral("--launcher-pid"),
            QString::number(launcherPid),
            QStringLiteral("--restart"),
            QStringLiteral("--lang"),
            langCode
        };
        // Try non-elevated launch first (portable tool, no UAC prompt).
        // Fall back to elevated launch if non-elevated fails (e.g. running from
        // a protected directory like C:\Program Files).
        if (processRunner->startNonElevated(updaterPath, arguments, &errorMessage)) {
            return true;
        }
        spdlog::info("PlatformUpdateInstallStrategy: non-elevated launch failed, "
                      "falling back to elevated launch: {}",
                      errorMessage.toStdString());
        return processRunner->startElevated(updaterPath, arguments, &errorMessage);
    }
};

// P2-38: This strategy is a deliberate cross-platform fallback, not dead code.
// The application is currently Windows-only, but the update subsystem keeps a
// platform abstraction so that a future Linux/macOS port only needs to provide
// a real install strategy here. Until then every method returns a localised
// "not supported" error so the UI can surface a clear message instead of
// silently no-op'ing. UpdateManager also falls back to DownloadOnly when no
// strategy is configured (null installStrategy_), so this class documents the
// contract that callers can always obtain an installMode() without null checks.
class DownloadOnlyInstallStrategy final : public core::update::PlatformUpdateInstallStrategy {
public:
    [[nodiscard]] core::update::UpdateInstallMode installMode(
        const infra::platform::IPlatformProcessRunner* processRunner) const noexcept override
    {
        Q_UNUSED(processRunner);
        return core::update::UpdateInstallMode::DownloadOnly;
    }

    [[nodiscard]] bool createInstallArtifact(const core::update::PreparedUpdateContext& context,
                                             QString& installArtifactPath,
                                             QString& errorMessage) const override
    {
        Q_UNUSED(context);
        Q_UNUSED(installArtifactPath);
        errorMessage = QCoreApplication::translate("core::update::UpdateManager",
                                                   "Automatic update is not supported on %1")
                           .arg(infra::platform::platformDisplayName());
        return false;
    }

    [[nodiscard]] bool launchInstallArtifact(const QString& installArtifactPath,
                                             const QString& langCode,
                                             infra::platform::IPlatformProcessRunner* processRunner,
                                             QString& errorMessage) const override
    {
        Q_UNUSED(installArtifactPath);
        Q_UNUSED(langCode);
        Q_UNUSED(processRunner);
        errorMessage = QCoreApplication::translate("core::update::UpdateManager",
                                                   "Automatic update is not supported on %1")
                           .arg(infra::platform::platformDisplayName());
        return false;
    }
};

} // namespace

namespace core::update {

QJsonObject PlatformUpdateInstallStrategy::buildWindowsTaskDocument(const PreparedUpdateContext& context)
{
    const QString backupExePath = context.applicationFilePath + QStringLiteral(".bak");

    QJsonObject root;
    root.insert(QStringLiteral("schemaVersion"), 1);
    root.insert(QStringLiteral("launcherPid"),
                static_cast<qint64>(QCoreApplication::applicationPid()));
    root.insert(QStringLiteral("targetExePath"),
                QDir::toNativeSeparators(context.applicationFilePath));
    root.insert(QStringLiteral("newExePath"),
                QDir::toNativeSeparators(context.updateFilePath));
    root.insert(QStringLiteral("backupExePath"),
                QDir::toNativeSeparators(backupExePath));
    root.insert(QStringLiteral("expectedVersion"), context.latestVersion);
    root.insert(QStringLiteral("expectedSha256"), context.expectedSha256);
    root.insert(QStringLiteral("restartAfterUpdate"), true);
    return root;
}

std::unique_ptr<PlatformUpdateInstallStrategy> createPlatformUpdateInstallStrategy()
{
    if (currentPackagePlatform().startsWith(QStringLiteral("windows-"))) {
        return std::make_unique<WindowsUpdateInstallStrategy>();
    }
    return std::make_unique<DownloadOnlyInstallStrategy>();
}

} // namespace core::update
