#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../../core/common/SettingsController.h"
#include "../../../ui/application/AppLifecycleCoordinator.h"
#include "../../../ui/application/LanguageCoordinator.h"
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
        languageCoordinator_ = std::make_unique<ui::application::LanguageCoordinator>(settingsController_.get());
        coordinator_ = std::make_unique<ui::application::AppLifecycleCoordinator>(
            &view_, settingsController_.get(), languageCoordinator_.get(), updateCoordinator_.get());

        settingsController_->setDisclaimerAccepted(true);
    }

    tests::mocks::FakeSettingsService settingsService_;
    tests::mocks::FakeMainWindowView view_;
    tests::mocks::FakeUpdateChecker updateChecker_;
    tests::mocks::FakeUpdateManager updateManager_;
    std::unique_ptr<core::common::SettingsController> settingsController_;
    std::unique_ptr<tests::mocks::MockUpdateCoordinator> updateCoordinator_;
    std::unique_ptr<ui::application::LanguageCoordinator> languageCoordinator_;
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

TEST_F(AppLifecycleCoordinatorTest, InitializeRestoresStateAndFinalizesStartup) {
    settingsController_->setMainWindowGeometry(QByteArrayLiteral("geometry"));
    settingsController_->setMainWindowState(QByteArrayLiteral("state"));
    settingsController_->setNavigationCollapsed(true);
    settingsController_->setModbusSettings(1500, 3, 200, true);
    settingsController_->setLanguage(QStringLiteral("zh_CN"));

    EXPECT_CALL(*updateCoordinator_, setCurrentLocale(QStringLiteral("zh_CN")));
    EXPECT_CALL(*updateCoordinator_, refreshIndicators());
    EXPECT_CALL(*updateCoordinator_, triggerAutoCheckIfNeeded());

    coordinator_->initialize();

    EXPECT_EQ(view_.initializeUiCallCount, 1);
    EXPECT_EQ(view_.restoreWindowGeometryCallCount, 1);
    EXPECT_EQ(view_.restoreWindowStateCallCount, 1);
    EXPECT_EQ(view_.restoredGeometry, QByteArrayLiteral("geometry"));
    EXPECT_EQ(view_.restoredState, QByteArrayLiteral("state"));
    EXPECT_TRUE(view_.navigationCollapsed);
    EXPECT_EQ(view_.applyModbusSettingsCallCount, 1);
    EXPECT_EQ(view_.appliedTimeoutMs, 1500);
    EXPECT_EQ(view_.appliedRetries, 3);
    EXPECT_EQ(view_.appliedRetryIntervalMs, 200);
    EXPECT_EQ(view_.retranslateUiCallCount, 1);
    EXPECT_EQ(view_.lastRetranslatedLocale, QStringLiteral("zh_CN"));
}

TEST_F(AppLifecycleCoordinatorTest, LanguageSelectionUpdatesCoordinatorAndRetranslatesView) {
    EXPECT_CALL(*updateCoordinator_, setCurrentLocale(QStringLiteral("zh_CN")));

    coordinator_->onLanguageSelected(QStringLiteral("zh_CN"));

    EXPECT_EQ(settingsController_->language(), QStringLiteral("zh_CN"));
    EXPECT_EQ(view_.retranslateUiCallCount, 1);
    EXPECT_EQ(view_.lastRetranslatedLocale, QStringLiteral("zh_CN"));
}

TEST_F(AppLifecycleCoordinatorTest, CheckForUpdatesEntryDelegatesToCoordinator) {
    EXPECT_CALL(*updateCoordinator_, checkForUpdates());

    coordinator_->onCheckForUpdatesRequested();
}

} // namespace
