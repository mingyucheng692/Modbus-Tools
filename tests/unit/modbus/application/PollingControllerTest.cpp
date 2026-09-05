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

TEST_F(PollingControllerTest, HandleTransientDisconnect_DoesNotStopActivePolling) {
    QSignalSpy stopSpy(controller_.get(), &PollingController::stopRequested);
    QSignalSpy submitSpy(controller_.get(), &PollingController::submitPollRequest);

    controller_->handleSessionConnected();
    const auto spec = makePollSpec();
    controller_->handlePollRequest(spec);
    processStateMachine();
    ASSERT_EQ(controller_->currentState(), PollState::Polling);
    ASSERT_EQ(submitSpy.count(), 1);

    // Transient disconnect occurs mid-session
    controller_->handleTransientDisconnect(QStringLiteral("tcp closed"));
    processStateMachine();

    // Should NOT stop poll or transition to Idle
    EXPECT_EQ(stopSpy.count(), 0);
    EXPECT_FALSE(controller_->context().sessionConnected);
    EXPECT_NE(controller_->currentState(), PollState::Idle);

    // Complete in-flight request with error
    controller_->handleResponse(false, 0, 0, QStringLiteral("closed"));
    processStateMachine();
    EXPECT_EQ(controller_->currentState(), PollState::Escalated);

    // Next periodic poll request is STILL submitted to drive self-healing
    controller_->handlePollRequest(spec);
    processStateMachine();
    EXPECT_EQ(submitSpy.count(), 2);
}

TEST_F(PollingControllerTest, TransientDisconnect_RecoversOnReconnection) {
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();
    controller_->handlePollRequest(spec);
    controller_->handleResponse(true, 10, 0, QString());
    ASSERT_EQ(controller_->currentState(), PollState::Polling);

    // Transient fault
    controller_->handleTransientDisconnect(QStringLiteral("link lost"));
    controller_->handlePollRequest(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("link lost"));
    EXPECT_EQ(controller_->currentState(), PollState::Escalated);

    // Reconnection signal arrives from core
    controller_->handleSessionConnected();
    EXPECT_TRUE(controller_->context().sessionConnected);

    // Next poll request succeeds -> recovers to Polling
    controller_->handlePollRequest(spec);
    controller_->handleResponse(true, 15, 0, QString());
    EXPECT_EQ(controller_->currentState(), PollState::Polling);
    EXPECT_EQ(controller_->context().consecutiveErrorCount, 0);
}

TEST_F(PollingControllerTest, TransientDisconnect_WithinHealingWindow_NoFatalSignal) {
    QSignalSpy fatalSpy(controller_.get(), &PollingController::pollingFatalDisconnect);

    // Default 30 s self-healing window: an Escalated fault right after the
    // transient disconnect must NOT be declared fatal yet.
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();
    controller_->handlePollRequest(spec);
    controller_->handleTransientDisconnect(QStringLiteral("link lost"));
    controller_->handleResponse(false, 0, 0, QStringLiteral("link lost"));

    EXPECT_EQ(controller_->currentState(), PollState::Escalated);
    EXPECT_EQ(fatalSpy.count(), 0);
}

TEST_F(PollingControllerTest, TransientDisconnect_HealingWindowExceeded_DeclaresFatalOnce) {
    QSignalSpy fatalSpy(controller_.get(), &PollingController::pollingFatalDisconnect);

    // Expire the self-healing window immediately.
    controller_->setFatalDisconnectTimeoutMs(0);
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();
    controller_->handlePollRequest(spec);
    controller_->handleTransientDisconnect(QStringLiteral("link lost"));
    controller_->handleResponse(false, 0, 0, QStringLiteral("link lost"));

    EXPECT_EQ(controller_->currentState(), PollState::Escalated);
    ASSERT_EQ(fatalSpy.count(), 1);

    // Guarded: further failures in the same fault window do not re-emit.
    controller_->handlePollRequest(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("link lost"));
    EXPECT_EQ(fatalSpy.count(), 1);

    // Recovery re-arms the notifier for the next fault window.
    controller_->handleSessionConnected();
    controller_->handleTransientDisconnect(QStringLiteral("link lost again"));
    controller_->handlePollRequest(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("link lost again"));
    EXPECT_EQ(fatalSpy.count(), 2);
}

TEST_F(PollingControllerTest, PersistentFailures_WhileSessionConnected_NeverDeclareFatal) {
    // Fatal is reserved for connection faults; protocol-level failures (CRC
    // errors, slave exceptions) must not trigger a teardown even in Escalated.
    QSignalSpy fatalSpy(controller_.get(), &PollingController::pollingFatalDisconnect);

    controller_->setFatalDisconnectTimeoutMs(0);
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();
    for (int i = 0; i < 6; ++i) {
        controller_->handlePollRequest(spec);
        controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    }

    EXPECT_EQ(controller_->currentState(), PollState::Escalated);
    EXPECT_EQ(fatalSpy.count(), 0);
}

TEST_F(PollingControllerTest, DegradedState_IdenticalErrorsRateLimitedToOncePerFiveSeconds) {
    QSignalSpy trafficSpy(controller_.get(), &PollingController::trafficEvent);

    controller_->setPollingInterval(100);
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();

    // 1st failure enters Degraded state and emits trafficEvent immediately
    controller_->handlePollRequest(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Degraded);
    EXPECT_EQ(trafficSpy.count(), 1);

    // Rapid successive identical failures within the Degraded window (consecutiveErrorCount stays below threshold=10)
    for (int i = 0; i < 5; ++i) {
        controller_->handlePollRequest(spec);
        controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
        processStateMachine();
    }

    // Still in Degraded (consecutiveErrorCount is 6 < threshold=10), warning must be suppressed by 5s rate limiter
    EXPECT_EQ(controller_->currentState(), PollState::Degraded);
    EXPECT_EQ(trafficSpy.count(), 1);
}

TEST_F(PollingControllerTest, DegradedState_DifferentiatingErrorsReportImmediately) {
    QSignalSpy trafficSpy(controller_.get(), &PollingController::trafficEvent);

    controller_->handleSessionConnected();
    const auto spec = makePollSpec();

    // 1st failure: crc error enters Degraded
    controller_->handlePollRequest(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Degraded);
    EXPECT_EQ(trafficSpy.count(), 1);

    // 2nd failure with a different error text should bypass rate limiter and report immediately
    controller_->handlePollRequest(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("parity error"));
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Degraded);
    EXPECT_EQ(trafficSpy.count(), 2);
}

} // namespace
