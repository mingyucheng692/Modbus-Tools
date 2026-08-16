#pragma once

#include "../../ui/application/UpdateCoordinator.h"

#include <gmock/gmock.h>

namespace tests::mocks {

class MockUpdateCoordinator : public ui::application::UpdateCoordinator {
public:
    MockUpdateCoordinator(ui::application::IUpdateInteractionView* view,
                          std::function<void()> requestQuit,
                          ui::common::UpdateChecker* updateChecker,
                          core::update::UpdateManager* updateManager,
                          core::common::SettingsController* settingsController,
                          QObject* parent = nullptr)
        : ui::application::UpdateCoordinator(view, std::move(requestQuit), updateChecker,
                                             updateManager, settingsController, parent) {}

    MOCK_METHOD(void, setCurrentLocale, (const QString& locale), (override));
    MOCK_METHOD(void, checkForUpdates, (), (override));
    MOCK_METHOD(void, triggerAutoCheckIfNeeded, (), (override));
    MOCK_METHOD(void, refreshIndicators, (), (override));
};

} // namespace tests::mocks
