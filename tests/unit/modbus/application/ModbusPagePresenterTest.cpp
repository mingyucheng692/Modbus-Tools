// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ui/application/modbus/ModbusPagePresenter.h"
#include "ui/application/modbus/ModbusSessionPresenter.h"
#include "ui/application/modbus/PollingController.h"
#include "ui/application/modbus/RequestSubmissionService.h"
#include "ui/application/modbus/TrafficLogController.h"
#include "mocks/MockModbusPageView.h"

using namespace testing;
using namespace ui::application::modbus;

namespace {

class ModbusPagePresenterTest : public Test {
protected:
    void SetUp() override {
        mockView_ = std::make_unique<NiceMock<tests::mocks::MockModbusPageView>>();
        presenter_ = std::make_unique<ModbusPagePresenter>(mockView_.get(), SessionMode::Tcp);
    }

    void TearDown() override {
        if (presenter_) {
            presenter_->detachView();
            presenter_.reset();
        }
        mockView_.reset();
    }

    std::unique_ptr<NiceMock<tests::mocks::MockModbusPageView>> mockView_;
    std::unique_ptr<ModbusPagePresenter> presenter_;
};

} // namespace

TEST_F(ModbusPagePresenterTest, ExecuteRequest_RejectsWhenDisconnected) {
    EXPECT_CALL(*mockView_, showStatusMessage(_, true)).Times(AtLeast(1));

    ModbusExecuteSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 10;
    spec.quantity = 2;

    presenter_->executeRequest(spec);
}

TEST_F(ModbusPagePresenterTest, TogglePolling_StopsPollingWhenFalse) {
    EXPECT_NO_THROW(presenter_->togglePolling(false, ModbusExecuteSpec{}, false));
    EXPECT_FALSE(presenter_->pollingController()->isPollingActive());
}

TEST_F(ModbusPagePresenterTest, TogglePolling_RejectsWritePollingWithoutConfirmation) {
    EXPECT_CALL(*mockView_, showStatusMessage(_, true)).Times(AtLeast(1));

    ModbusExecuteSpec spec;
    spec.functionCode = 0x06;
    spec.startAddress = 0;
    spec.quantity = 1;
    spec.writeData = QByteArray::fromHex("0001");

    presenter_->togglePolling(true, spec, false);
    EXPECT_FALSE(presenter_->pollingController()->isPollingActive());
}

TEST_F(ModbusPagePresenterTest, TogglePolling_RejectsBroadcastWritePolling) {
    auto rtuPresenter = std::make_unique<ModbusPagePresenter>(mockView_.get(), SessionMode::Rtu);

    EXPECT_CALL(*mockView_, showStatusMessage(_, true)).Times(AtLeast(1));

    ModbusExecuteSpec spec;
    spec.slaveId = 0;
    spec.functionCode = 0x06;
    spec.startAddress = 0;
    spec.quantity = 1;
    spec.writeData = QByteArray::fromHex("0001");

    rtuPresenter->togglePolling(true, spec, true);
    EXPECT_FALSE(rtuPresenter->pollingController()->isPollingActive());

    rtuPresenter->detachView();
}

TEST_F(ModbusPagePresenterTest, ChangePollInterval_UpdatesPollingController) {
    presenter_->changePollInterval(250);
    // Polling controller interval is set
    EXPECT_NO_THROW(presenter_->changePollInterval(500));
}

TEST_F(ModbusPagePresenterTest, HandleLogMessage_RoutesWithoutError) {
    EXPECT_NO_THROW(presenter_->handleLogMessage(QStringLiteral("info log"), false));
    EXPECT_NO_THROW(presenter_->handleLogMessage(QStringLiteral("error log"), true));
}

TEST_F(ModbusPagePresenterTest, SetRawFramesEnabled_UpdatesTrafficLogController) {
    presenter_->setRawFramesEnabled(true);
    EXPECT_TRUE(presenter_->trafficLogController()->isRawFramesEnabled());

    presenter_->setRawFramesEnabled(false);
    EXPECT_FALSE(presenter_->trafficLogController()->isRawFramesEnabled());
}

TEST_F(ModbusPagePresenterTest, DetachView_NullifiesView) {
    EXPECT_EQ(presenter_->view(), mockView_.get());
    presenter_->detachView();
    EXPECT_EQ(presenter_->view(), nullptr);
}

