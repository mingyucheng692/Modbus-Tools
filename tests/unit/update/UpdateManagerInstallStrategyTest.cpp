#include <gtest/gtest.h>

#include "../../../core/update/PlatformUpdateInstallStrategy.h"
#include "../../../core/update/UpdateManager.h"

namespace {

class FakeProcessRunner final : public infra::platform::IPlatformProcessRunner {
public:
    [[nodiscard]] bool supportsElevatedLaunch() const noexcept override
    {
        return supportsElevatedLaunch_;
    }

    [[nodiscard]] bool startElevated(const QString& executablePath,
                                     const QStringList& arguments,
                                     QString* errorMessage) override
    {
        lastExecutablePath_ = executablePath;
        lastArguments_ = arguments;
        if (errorMessage != nullptr) {
            *errorMessage = errorMessage_;
        }
        return startElevatedResult_;
    }

    bool supportsElevatedLaunch_ = false;
    bool startElevatedResult_ = false;
    QString errorMessage_;
    QString lastExecutablePath_;
    QStringList lastArguments_;
};

class FakeInstallStrategy final : public core::update::PlatformUpdateInstallStrategy {
public:
    [[nodiscard]] core::update::UpdateInstallMode installMode(
        const infra::platform::IPlatformProcessRunner* processRunner) const noexcept override
    {
        lastInstallModeProcessRunner_ = processRunner;
        return installMode_;
    }

    [[nodiscard]] bool createInstallArtifact(const core::update::PreparedUpdateContext& context,
                                             QString& installArtifactPath,
                                             QString& errorMessage) const override
    {
        lastPreparedContext_ = context;
        installArtifactPath = installArtifactPath_;
        errorMessage = createArtifactError_;
        return createArtifactResult_;
    }

    [[nodiscard]] bool launchInstallArtifact(const QString& installArtifactPath,
                                             const QString& langCode,
                                             infra::platform::IPlatformProcessRunner* processRunner,
                                             QString& errorMessage) const override
    {
        lastLaunchInstallArtifactPath_ = installArtifactPath;
        lastLaunchLangCode_ = langCode;
        lastLaunchProcessRunner_ = processRunner;
        errorMessage = launchError_;
        return launchResult_;
    }

    mutable const infra::platform::IPlatformProcessRunner* lastInstallModeProcessRunner_ = nullptr;
    mutable core::update::PreparedUpdateContext lastPreparedContext_;
    mutable QString lastLaunchInstallArtifactPath_;
    mutable QString lastLaunchLangCode_;
    mutable infra::platform::IPlatformProcessRunner* lastLaunchProcessRunner_ = nullptr;
    core::update::UpdateInstallMode installMode_ = core::update::UpdateInstallMode::DownloadOnly;
    bool createArtifactResult_ = true;
    bool launchResult_ = false;
    QString installArtifactPath_ = QStringLiteral("prepared-artifact");
    QString createArtifactError_;
    QString launchError_;
};

} // namespace

TEST(UpdateManagerInstallStrategy, InstallModeDelegatesToInjectedStrategy)
{
    auto processRunner = std::make_unique<FakeProcessRunner>();
    FakeProcessRunner* processRunnerPtr = processRunner.get();

    auto installStrategy = std::make_unique<FakeInstallStrategy>();
    installStrategy->installMode_ = core::update::UpdateInstallMode::AutomaticInstaller;
    FakeInstallStrategy* installStrategyPtr = installStrategy.get();

    core::update::UpdateManager manager(nullptr, std::move(processRunner), std::move(installStrategy));

    EXPECT_EQ(manager.installMode(), core::update::UpdateInstallMode::AutomaticInstaller);
    EXPECT_EQ(installStrategyPtr->lastInstallModeProcessRunner_, processRunnerPtr);
}

TEST(UpdateManagerInstallStrategy, LaunchInstallerDelegatesToInjectedStrategy)
{
    auto processRunner = std::make_unique<FakeProcessRunner>();
    FakeProcessRunner* processRunnerPtr = processRunner.get();

    auto installStrategy = std::make_unique<FakeInstallStrategy>();
    installStrategy->launchResult_ = true;
    FakeInstallStrategy* installStrategyPtr = installStrategy.get();

    core::update::UpdateManager manager(nullptr, std::move(processRunner), std::move(installStrategy));

    QString errorMessage;
    EXPECT_TRUE(manager.launchInstaller(QStringLiteral("prepared-task.json"),
                                        QStringLiteral("zh_CN"),
                                        errorMessage));
    EXPECT_EQ(installStrategyPtr->lastLaunchInstallArtifactPath_, QStringLiteral("prepared-task.json"));
    EXPECT_EQ(installStrategyPtr->lastLaunchLangCode_, QStringLiteral("zh_CN"));
    EXPECT_EQ(installStrategyPtr->lastLaunchProcessRunner_, processRunnerPtr);
}
