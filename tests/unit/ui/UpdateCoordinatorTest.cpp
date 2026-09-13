#include <gtest/gtest.h>

#include "../../../core/Config.h"
#include "../../../core/common/SettingsController.h"
#include "../../../ui/application/UpdateCoordinator.h"
#include "../../mocks/UiTestDoubles.h"

namespace {

class UpdateCoordinatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        settingsController_ = std::make_unique<core::common::SettingsController>(&settingsService_);
        coordinator_ = std::make_unique<ui::application::UpdateCoordinator>(
            &view_,
            [this] { ++requestQuitCallCount; },
            &updateChecker_, &updateManager_, settingsController_.get());
    }

    int requestQuitCallCount = 0;

    tests::mocks::FakeSettingsService settingsService_;
    tests::mocks::FakeMainWindowView view_;
    tests::mocks::FakeUpdateChecker updateChecker_;
    tests::mocks::FakeUpdateManager updateManager_;
    std::unique_ptr<core::common::SettingsController> settingsController_;
    std::unique_ptr<ui::application::UpdateCoordinator> coordinator_;
};

TEST_F(UpdateCoordinatorTest, ManualCheckDisablesActionAndStartsCheck) {
    coordinator_->checkForUpdates();

    EXPECT_EQ(updateChecker_.checkForUpdatesCallCount, 1);
    EXPECT_FALSE(view_.updateCheckActionEnabled);
    EXPECT_FALSE(settingsController_->lastUpdateCheckUtc().isEmpty());
}

TEST_F(UpdateCoordinatorTest, AutoCheckSkipsWhenFrequencyIsNever) {
    settingsController_->setUpdateCheckFrequency(config::App::kUpdateCheckNever);

    coordinator_->triggerAutoCheckIfNeeded();

    EXPECT_EQ(updateChecker_.checkForUpdatesCallCount, 0);
    EXPECT_TRUE(settingsController_->lastUpdateCheckUtc().isEmpty());
}

TEST_F(UpdateCoordinatorTest, UpdateAvailableBranchShowsPromptAndIndicator) {
    view_.promptChoice = ui::application::UpdatePromptChoice::Cancel;

    coordinator_->checkForUpdates();
    updateChecker_.emitUpdateAvailable(QStringLiteral("1.0.0"),
                                       QStringLiteral("1.1.0"),
                                       QStringLiteral("https://example.com/update-only.zip"),
                                       QStringLiteral("sha256"),
                                       QStringLiteral("https://example.com/checksums.txt"),
                                       QStringLiteral("https://example.com/setup.exe"),
                                       QStringLiteral("https://example.com/releases"));

    EXPECT_TRUE(coordinator_->updateAvailable());
    EXPECT_TRUE(view_.updateCheckActionEnabled);
    EXPECT_TRUE(view_.updateIndicatorVisible);
    EXPECT_EQ(view_.promptUpdateActionCallCount, 1);
    EXPECT_EQ(view_.lastPromptCurrentVersion, QStringLiteral("1.0.0"));
    EXPECT_EQ(view_.lastPromptLatestVersion, QStringLiteral("1.1.0"));
}

TEST_F(UpdateCoordinatorTest, DownloadOnlyPlatformShowsExplicitDownloadFlow) {
    updateManager_.installMode_ = core::update::UpdateInstallMode::DownloadOnly;
    view_.confirmOpenDownloadPageResult = false;

    coordinator_->checkForUpdates();
    updateChecker_.emitUpdateAvailable(QStringLiteral("1.0.0"),
                                       QStringLiteral("1.1.0"),
                                       QStringLiteral("https://example.com/update-only.zip"),
                                       QStringLiteral("sha256"),
                                       QStringLiteral("https://example.com/checksums.txt"),
                                       QStringLiteral("https://example.com/setup.dmg"),
                                       QStringLiteral("https://example.com/releases"));

    EXPECT_TRUE(coordinator_->updateAvailable());
    EXPECT_EQ(view_.showUpdateInfoMessageCallCount, 1);
    EXPECT_EQ(view_.confirmOpenDownloadPageCallCount, 1);
    EXPECT_EQ(view_.promptUpdateActionCallCount, 0);
}

TEST_F(UpdateCoordinatorTest, TerminalCommandGuidanceShowsCommandDialogInsteadOfConfirm) {
    view_.confirmOpenDownloadPageResult = true;

    coordinator_->checkForUpdates();
    updateChecker_.emitUpdateAvailable(QStringLiteral("1.0.0"),
                                       QStringLiteral("1.1.0"),
                                       QString(),  // no UpdateOnly asset on Linux
                                       QString(),
                                       QString(),
                                       QStringLiteral("https://example.com/Modbus-Tools-v1.1.0-linux-x86_64.tar.gz"),
                                       QStringLiteral("https://example.com/releases/tag/v1.1.0"),
                                       core::update::UpdateGuidance::TerminalCommand,
                                       QStringLiteral("abc123"));

    EXPECT_TRUE(coordinator_->updateAvailable());
    EXPECT_EQ(view_.showUpdateCommandDialogCallCount, 1);
    EXPECT_EQ(view_.lastCommandDialogLatestVersion, QStringLiteral("1.1.0"));
    EXPECT_TRUE(view_.lastCommandDialogCommand.contains(
        QStringLiteral("https://example.com/Modbus-Tools-v1.1.0-linux-x86_64.tar.gz")));
    EXPECT_TRUE(view_.lastCommandDialogCommand.contains(QStringLiteral("sha256sum -c")));
    EXPECT_TRUE(view_.lastCommandDialogCommand.contains(QStringLiteral("abc123")));
    EXPECT_EQ(view_.lastCommandDialogReleaseUrl, QStringLiteral("https://example.com/releases/tag/v1.1.0"));
    // The command flow replaces — never stacks with — the download-page confirm.
    EXPECT_EQ(view_.confirmOpenDownloadPageCallCount, 0);
    EXPECT_EQ(view_.promptUpdateActionCallCount, 0);
}

TEST_F(UpdateCoordinatorTest, OpenDownloadPageGuidanceKeepsHistoricalConfirmFlow) {
    view_.confirmOpenDownloadPageResult = true;

    coordinator_->checkForUpdates();
    updateChecker_.emitUpdateAvailable(QStringLiteral("1.0.0"),
                                       QStringLiteral("1.1.0"),
                                       QString(),
                                       QString(),
                                       QString(),
                                       QStringLiteral("https://example.com/Modbus-Tools-v1.1.0-macos-arm64.dmg"),
                                       QStringLiteral("https://example.com/releases/tag/v1.1.0"),
                                       core::update::UpdateGuidance::OpenDownloadPage);

    EXPECT_EQ(view_.showUpdateCommandDialogCallCount, 0);
    EXPECT_EQ(view_.confirmOpenDownloadPageCallCount, 1);
}

TEST_F(UpdateCoordinatorTest, TerminalCommandWithoutPackageAssetDegradesToDownloadPage) {
    // Older releases may carry no recognizable Linux asset: fullPackageUrl
    // falls back to the release page itself — the terminal flow has nothing
    // to download and must degrade to the historical behavior.
    view_.confirmOpenDownloadPageResult = true;

    coordinator_->checkForUpdates();
    updateChecker_.emitUpdateAvailable(QStringLiteral("1.0.0"),
                                       QStringLiteral("1.1.0"),
                                       QString(),
                                       QString(),
                                       QString(),
                                       QStringLiteral("https://example.com/releases/tag/v1.1.0"),
                                       QStringLiteral("https://example.com/releases/tag/v1.1.0"),
                                       core::update::UpdateGuidance::TerminalCommand);

    EXPECT_EQ(view_.showUpdateCommandDialogCallCount, 0);
    EXPECT_EQ(view_.confirmOpenDownloadPageCallCount, 1);
}

TEST_F(UpdateCoordinatorTest, TerminalCommandWithMissingDigestStillShowsCommandDialog) {
    // Empty sha256 is surfaced by the dialog as an explicit WARNING segment,
    // not as a silent skip — the command flow remains usable.
    coordinator_->checkForUpdates();
    updateChecker_.emitUpdateAvailable(QStringLiteral("1.0.0"),
                                       QStringLiteral("1.1.0"),
                                       QString(),
                                       QString(),
                                       QString(),
                                       QStringLiteral("https://example.com/Modbus-Tools-v1.1.0-linux-x86_64.tar.gz"),
                                       QStringLiteral("https://example.com/releases/tag/v1.1.0"),
                                       core::update::UpdateGuidance::TerminalCommand,
                                       QString() /* no digest published */);

    EXPECT_EQ(view_.showUpdateCommandDialogCallCount, 1);
    EXPECT_TRUE(view_.lastCommandDialogCommand.contains(QStringLiteral("WARNING")));
    EXPECT_FALSE(view_.lastCommandDialogCommand.contains(QStringLiteral("sha256sum")));
}

TEST_F(UpdateCoordinatorTest, DownloadFailureDuringManualCheckShowsWarning) {
    coordinator_->checkForUpdates();
    updateManager_.emitUpdateFailed(QStringLiteral("Download failed"));

    EXPECT_EQ(view_.showUpdateWarningMessageCallCount, 1);
    EXPECT_EQ(view_.lastWarningMessage, QStringLiteral("Download failed"));
}

} // namespace
