#include <gtest/gtest.h>

#include "../../../core/AppConstants.h"
#include "../../../core/common/SettingsController.h"
#include "../../../ui/application/UpdateCoordinator.h"
#include "../../mocks/UiTestDoubles.h"

namespace {

class UpdateCoordinatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        settingsController_ = std::make_unique<core::common::SettingsController>(&settingsService_);
        coordinator_ = std::make_unique<ui::application::UpdateCoordinator>(
            &view_, &updateChecker_, &updateManager_, settingsController_.get());
    }

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
    settingsController_->setUpdateCheckFrequency(app::constants::Values::App::kUpdateCheckNever);

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

TEST_F(UpdateCoordinatorTest, DownloadFailureDuringManualCheckShowsWarning) {
    coordinator_->checkForUpdates();
    updateManager_.emitUpdateFailed(QStringLiteral("Download failed"));

    EXPECT_EQ(view_.showUpdateWarningMessageCallCount, 1);
    EXPECT_EQ(view_.lastWarningMessage, QStringLiteral("Download failed"));
}

} // namespace
