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

        const QStringList arguments{
            QStringLiteral("--task"),
            QDir::toNativeSeparators(installArtifactPath),
            QStringLiteral("--lang"),
            langCode
        };
        return processRunner->startElevated(updaterPath, arguments, &errorMessage);
    }
};

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
