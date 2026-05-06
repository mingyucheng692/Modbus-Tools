#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../../core/common/SettingsController.h"
#include "../../../ui/application/AppLifecycleCoordinator.h"
#include "../../mocks/MockUpdateCoordinator.h"
#include "../../mocks/UiTestDoubles.h"

using namespace testing;

namespace {

class AppLifecycleCoordinatorTest : public Test {
protected:
    void SetUp() override {
        settingsController_ = std::make_unique<core::common::SettingsController>(&settingsService_);
        updateCoordinator_ = std::make_unique<tests::mocks::MockUpdateCoordinator>(
            &view_, &updateChecker_, &updateManager_, settingsController_.get());
        coordinator_ = std::make_unique<ui::application::AppLifecycleCoordinator>(
            &view_, settingsController_.get(), updateCoordinator_.get());
    }

    tests::mocks::FakeSettingsService settingsService_;
    tests::mocks::FakeMainWindowView view_;
    tests::mocks::FakeUpdateChecker updateChecker_;
    tests::mocks::FakeUpdateManager updateManager_;
    std::unique_ptr<core::common::SettingsController> settingsController_;
    std::unique_ptr<tests::mocks::MockUpdateCoordinator> updateCoordinator_;
    std::unique_ptr<ui::application::AppLifecycleCoordinator> coordinator_;
};

TEST_F(AppLifecycleCoordinatorTest, DisclaimerAcceptedFlowPersistsAcceptance) {
    settingsController_->setDisclaimerAccepted(false);
    view_.disclaimerDialogResult = true;

    EXPECT_CALL(*updateCoordinator_, refreshIndicators());
    EXPECT_CALL(*updateCoordinator_, triggerAutoCheckIfNeeded());

    coordinator_->finalizeStartup();

    EXPECT_EQ(view_.showDisclaimerDialogCallCount, 1);
    EXPECT_TRUE(settingsController_->disclaimerAccepted());
}

TEST_F(AppLifecycleCoordinatorTest, DisclaimerRejectedFlowRequestsQuit) {
    settingsController_->setDisclaimerAccepted(false);
    view_.disclaimerDialogResult = false;

    EXPECT_CALL(*updateCoordinator_, refreshIndicators());
    EXPECT_CALL(*updateCoordinator_, triggerAutoCheckIfNeeded());

    coordinator_->finalizeStartup();

    EXPECT_EQ(view_.showDisclaimerDialogCallCount, 1);
    EXPECT_FALSE(settingsController_->disclaimerAccepted());
}

TEST_F(AppLifecycleCoordinatorTest, PersistOnCloseStoresWindowStateAndSyncs) {
    view_.savedGeometry = QByteArrayLiteral("saved-geometry-bytes");
    view_.savedState = QByteArrayLiteral("saved-state-bytes");

    coordinator_->persistOnClose();

    EXPECT_EQ(settingsController_->mainWindowGeometry(), QByteArrayLiteral("saved-geometry-bytes"));
    EXPECT_EQ(settingsController_->mainWindowState(), QByteArrayLiteral("saved-state-bytes"));
    EXPECT_EQ(settingsService_.syncCount, 1);
}

} // namespace
