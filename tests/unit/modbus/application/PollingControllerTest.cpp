#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>

#include "../../../ui/application/modbus/PollingController.h"
#include "../../../ui/application/modbus/RequestSubmissionService.h"
#include "../../../ui/application/modbus/ModbusTypes.h"

using namespace ui::application::modbus;

namespace {

class PollingControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        requestService_ = std::make_unique<RequestSubmissionService>();
        controller_ = std::make_unique<PollingController>(requestService_.get());
    }

    PollSpec makePollSpec() const {
        PollSpec spec;
        spec.functionCode = 0x03;
        spec.startAddress = 0;
        spec.quantity = 1;
        spec.slaveId = 1;
        return spec;
    }

    void processStateMachine() {
        QCoreApplication::processEvents();
    }

    std::unique_ptr<RequestSubmissionService> requestService_;
    std::unique_ptr<PollingController> controller_;
};

TEST_F(PollingControllerTest, InitialState_UsesIdleDisconnectedContext) {
    EXPECT_EQ(controller_->currentState(), PollState::Idle);
    EXPECT_FALSE(controller_->context().sessionConnected);
    EXPECT_FALSE(controller_->context().requestInFlight);
}

TEST_F(PollingControllerTest, SetPollingInterval) {
    controller_->setPollingInterval(500);
}

TEST_F(PollingControllerTest, HandlePollRequest_NotConnected_DoesNotSubmit) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);

    controller_->handleSessionDisconnected();
    controller_->handlePollRequest(makePollSpec());

    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(controller_->currentState(), PollState::Idle);
}

TEST_F(PollingControllerTest, HandleSessionConnected_AllowsPollSubmission) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);
    QSignalSpy stateSpy(controller_.get(), &PollingController::stateChanged);

    controller_->handleSessionConnected();
    controller_->handlePollRequest(makePollSpec());
    processStateMachine();

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(controller_->currentState(), PollState::Polling);
    EXPECT_TRUE(controller_->context().sessionConnected);
    EXPECT_TRUE(controller_->context().requestInFlight);
    EXPECT_FALSE(stateSpy.isEmpty());
}

TEST_F(PollingControllerTest, HandlePollRequest_PollInFlight_IgnoresDuplicate) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);

    controller_->handleSessionConnected();
    const auto spec = makePollSpec();
    controller_->handlePollRequest(spec);
    controller_->handlePollRequest(spec);

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(PollingControllerTest, HandleSessionDisconnected_StopsActivePollAndClearsInFlight) {
    QSignalSpy stopSpy(controller_.get(), &PollingController::stopRequested);

    controller_->handleSessionConnected();
    controller_->handlePollRequest(makePollSpec());
    processStateMachine();
    ASSERT_TRUE(controller_->context().requestInFlight);

    controller_->handleSessionDisconnected(QStringLiteral("link down"));
    processStateMachine();

    EXPECT_GE(stopSpy.count(), 1);
    EXPECT_EQ(controller_->currentState(), PollState::Idle);
    EXPECT_FALSE(controller_->context().sessionConnected);
    EXPECT_FALSE(controller_->context().requestInFlight);
    EXPECT_FALSE(controller_->isSuppressingTrafficLog());
}

TEST_F(PollingControllerTest, ConsecutiveFailures_DegradeThenEscalate) {
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();

    controller_->handlePollRequest(spec);
    processStateMachine();
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    processStateMachine();
    EXPECT_EQ(controller_->currentState(), PollState::Degraded);

    controller_->handlePollRequest(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    controller_->handlePollRequest(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Escalated);
}

TEST_F(PollingControllerTest, SuccessAfterFailures_RecoversAndClearsErrorTracking) {
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();

    controller_->handlePollRequest(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("timeout"));
    processStateMachine();
    ASSERT_EQ(controller_->currentState(), PollState::Degraded);

    controller_->handlePollRequest(spec);
    controller_->handleResponse(true, 10, 1, QString());
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Polling);
    EXPECT_EQ(controller_->context().consecutiveErrorCount, 0);
}

TEST_F(PollingControllerTest, StopPoll_TransitionsToIdleAndClearsTracking) {
    controller_->handleSessionConnected();
    controller_->handlePollRequest(makePollSpec());
    processStateMachine();
    controller_->handleResponse(false, 0, 0, QStringLiteral("error"));
    processStateMachine();

    controller_->stopPoll();
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Idle);
    EXPECT_FALSE(controller_->context().requestInFlight);
    EXPECT_FALSE(controller_->isSuppressingTrafficLog());
    EXPECT_EQ(controller_->context().consecutiveErrorCount, 0);
}

TEST_F(PollingControllerTest, Reset_ClearsAllTrackingAndStops) {
    controller_->handleSessionConnected();
    controller_->handlePollRequest(makePollSpec());
    controller_->handleResponse(false, 0, 0, QStringLiteral("error"));

    controller_->reset();
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Idle);
    EXPECT_FALSE(controller_->context().requestInFlight);
    EXPECT_EQ(controller_->context().consecutiveErrorCount, 0);
}

TEST_F(PollingControllerTest, MultipleStopPolls_DoNotCrash) {
    controller_->handleSessionConnected();
    controller_->handlePollRequest(makePollSpec());
    processStateMachine();

    controller_->stopPoll();
    controller_->stopPoll();
    controller_->stopPoll();

    EXPECT_EQ(controller_->currentState(), PollState::Idle);
}

} // namespace
