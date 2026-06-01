#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <QSignalSpy>

#include "../../../ui/application/modbus/PollingController.h"
#include "../../../ui/application/modbus/RequestSubmissionService.h"
#include "../../../ui/application/modbus/ModbusTypes.h"

using namespace testing;
using namespace ui::application::modbus;

namespace {

class PollingControllerStateMachineTest : public Test {
protected:
    void SetUp() override {
        requestService_ = std::make_unique<RequestSubmissionService>();
        controller_ = std::make_unique<PollingController>(requestService_.get());
    }

    void submitPoll() {
        PollSpec spec;
        spec.functionCode = 0x03;
        spec.startAddress = 0;
        spec.quantity = 1;
        controller_->handlePollRequest(spec);
    }

    void failPoll(int count, const QString& error = QStringLiteral("error")) {
        for (int i = 0; i < count; ++i) {
            submitPoll();
            controller_->handleResponse(false, 0, 0, error);
        }
    }

    void succeedPoll() {
        submitPoll();
        controller_->handleResponse(true, 10, 0, QString());
    }

    std::unique_ptr<RequestSubmissionService> requestService_;
    std::unique_ptr<PollingController> controller_;
};

TEST_F(PollingControllerStateMachineTest, InitialState_IsIdle) {
    EXPECT_EQ(controller_->currentState(), PollState::Idle);
}

TEST_F(PollingControllerStateMachineTest, HandlePollRequest_NotConnected_DoesNotSubmit) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);

    controller_->setSessionConnected(false);
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 10;
    controller_->handlePollRequest(spec);

    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(controller_->currentState(), PollState::Idle);
}

TEST_F(PollingControllerStateMachineTest, HandlePollRequest_Connected_SubmitsRequest) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);

    controller_->setSessionConnected(true);
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 10;
    controller_->handlePollRequest(spec);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(PollingControllerStateMachineTest, HandlePollRequest_PollInFlight_IgnoresDuplicate) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);

    controller_->setSessionConnected(true);
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 10;

    controller_->handlePollRequest(spec);
    controller_->handlePollRequest(spec);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(PollingControllerStateMachineTest, StopPoll_TransitionsToIdle) {
    controller_->setSessionConnected(true);
    submitPoll();

    controller_->stopPoll();

    EXPECT_EQ(controller_->currentState(), PollState::Idle);
}

TEST_F(PollingControllerStateMachineTest, StopPoll_FromIdle_RemainsIdle) {
    controller_->stopPoll();

    EXPECT_EQ(controller_->currentState(), PollState::Idle);
}

TEST_F(PollingControllerStateMachineTest, Reset_ClearsAllTrackingAndStops) {
    controller_->setSessionConnected(true);
    submitPoll();
    controller_->handleResponse(false, 0, 0, QStringLiteral("error"));

    controller_->reset();

    EXPECT_EQ(controller_->currentState(), PollState::Idle);
}

TEST_F(PollingControllerStateMachineTest, MultipleStopPolls_DoNotCrash) {
    controller_->setSessionConnected(true);
    submitPoll();

    controller_->stopPoll();
    controller_->stopPoll();
    controller_->stopPoll();

    EXPECT_EQ(controller_->currentState(), PollState::Idle);
}

TEST_F(PollingControllerStateMachineTest, HandleResponse_Success_DoesNotCrash) {
    controller_->setSessionConnected(true);
    submitPoll();

    QSignalSpy summarySpy(controller_.get(), &PollingController::summaryReady);
    controller_->handleResponse(true, 10, 0, QString());

    summarySpy.wait(1000);
}

TEST_F(PollingControllerStateMachineTest, HandleResponse_Error_DoesNotCrash) {
    controller_->setSessionConnected(true);
    submitPoll();

    QSignalSpy summarySpy(controller_.get(), &PollingController::summaryReady);
    controller_->handleResponse(false, 0, 0, QStringLiteral("timeout"));

    summarySpy.wait(1000);
}

} // namespace
