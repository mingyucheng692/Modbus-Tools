// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>
#include <QTimer>

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

    PollSpec makePollSpec(int intervalMs = 1000) const {
        PollSpec spec;
        spec.functionCode = 0x03;
        spec.startAddress = 0;
        spec.quantity = 1;
        spec.slaveId = 1;
        spec.intervalMs = intervalMs;
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
    controller_->handleSessionConnected();
    controller_->startPolling(makePollSpec());
    auto* timer = controller_->findChild<QTimer*>();
    ASSERT_NE(timer, nullptr);
    EXPECT_TRUE(timer->isActive());

    controller_->setPollingInterval(500);
    EXPECT_EQ(timer->interval(), 500);
    controller_->stopPoll();
}

TEST_F(PollingControllerTest, StartPolling_NotConnected_DoesNotSubmit) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);

    controller_->handleSessionDisconnected();
    controller_->startPolling(makePollSpec());

    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(controller_->currentState(), PollState::Idle);
    controller_->stopPoll();
}

TEST_F(PollingControllerTest, HandleSessionConnected_AllowsPollSubmission) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);

    controller_->handleSessionConnected();
    controller_->startPolling(makePollSpec());
    processStateMachine();

    EXPECT_EQ(spy.count(), 1);
    EXPECT_EQ(controller_->currentState(), PollState::Polling);
    EXPECT_TRUE(controller_->context().sessionConnected);
    EXPECT_TRUE(controller_->context().requestInFlight);
}

TEST_F(PollingControllerTest, StartPolling_PollInFlight_IgnoresDuplicate) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);

    controller_->handleSessionConnected();
    const auto spec = makePollSpec();
    controller_->startPolling(spec);
    controller_->onPollTimerTick();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(PollingControllerTest, HandleSessionDisconnected_StopsActivePollAndClearsInFlight) {
    QSignalSpy stopSpy(controller_.get(), &PollingController::stopRequested);

    controller_->handleSessionConnected();
    controller_->startPolling(makePollSpec());
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

    controller_->startPolling(spec);
    processStateMachine();
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    processStateMachine();
    EXPECT_EQ(controller_->currentState(), PollState::Degraded);

    controller_->onPollTimerTick();
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    controller_->onPollTimerTick();
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Escalated);
}

TEST_F(PollingControllerTest, SuccessAfterFailures_RecoversAndClearsErrorTracking) {
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();

    controller_->startPolling(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("timeout"));
    processStateMachine();
    ASSERT_EQ(controller_->currentState(), PollState::Degraded);

    controller_->onPollTimerTick();
    controller_->handleResponse(true, 10, 1, QString());
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Polling);
    EXPECT_EQ(controller_->context().consecutiveErrorCount, 0);
}

TEST_F(PollingControllerTest, StopPoll_TransitionsToIdleAndClearsTracking) {
    controller_->handleSessionConnected();
    controller_->startPolling(makePollSpec());
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
    controller_->startPolling(makePollSpec());
    controller_->handleResponse(false, 0, 0, QStringLiteral("error"));

    controller_->reset();
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Idle);
    EXPECT_FALSE(controller_->context().requestInFlight);
    EXPECT_EQ(controller_->context().consecutiveErrorCount, 0);
}

TEST_F(PollingControllerTest, MultipleStopPolls_DoNotCrash) {
    controller_->handleSessionConnected();
    controller_->startPolling(makePollSpec());
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
    controller_->startPolling(spec);
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

    // Next periodic poll request is STILL submitted via tick to drive self-healing
    controller_->onPollTimerTick();
    processStateMachine();
    EXPECT_EQ(submitSpy.count(), 2);
}

TEST_F(PollingControllerTest, TransientDisconnect_RecoversOnReconnection) {
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();
    controller_->startPolling(spec);
    controller_->handleResponse(true, 10, 0, QString());
    ASSERT_EQ(controller_->currentState(), PollState::Polling);

    // Transient fault
    controller_->handleTransientDisconnect(QStringLiteral("link lost"));
    controller_->onPollTimerTick();
    controller_->handleResponse(false, 0, 0, QStringLiteral("link lost"));
    EXPECT_EQ(controller_->currentState(), PollState::Escalated);

    // Reconnection signal arrives from core
    controller_->handleSessionConnected();
    EXPECT_TRUE(controller_->context().sessionConnected);

    // Next poll request succeeds -> recovers to Polling
    controller_->onPollTimerTick();
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
    controller_->startPolling(spec);
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
    controller_->startPolling(spec);
    controller_->handleTransientDisconnect(QStringLiteral("link lost"));
    controller_->handleResponse(false, 0, 0, QStringLiteral("link lost"));

    EXPECT_EQ(controller_->currentState(), PollState::Escalated);
    ASSERT_EQ(fatalSpy.count(), 1);

    // Guarded: further failures in the same fault window do not re-emit.
    controller_->onPollTimerTick();
    controller_->handleResponse(false, 0, 0, QStringLiteral("link lost"));
    EXPECT_EQ(fatalSpy.count(), 1);

    // Recovery re-arms the notifier for the next fault window.
    controller_->handleSessionConnected();
    controller_->handleTransientDisconnect(QStringLiteral("link lost again"));
    controller_->onPollTimerTick();
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
    controller_->startPolling(spec);
    for (int i = 0; i < 6; ++i) {
        controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
        controller_->onPollTimerTick();
    }

    EXPECT_EQ(controller_->currentState(), PollState::Escalated);
    EXPECT_EQ(fatalSpy.count(), 0);
}

TEST_F(PollingControllerTest, DegradedState_IdenticalErrorsRateLimitedToOncePerFiveSeconds) {
    QSignalSpy trafficSpy(controller_.get(), &PollingController::trafficEvent);

    controller_->handleSessionConnected();
    const auto spec = makePollSpec(100);

    // 1st failure enters Degraded state and emits trafficEvent immediately
    controller_->startPolling(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Degraded);
    EXPECT_EQ(trafficSpy.count(), 1);

    // Rapid successive identical failures within the Degraded window
    for (int i = 0; i < 5; ++i) {
        controller_->onPollTimerTick();
        controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
        processStateMachine();
    }

    // Still in Degraded, warning must be suppressed by 5s rate limiter
    EXPECT_EQ(controller_->currentState(), PollState::Degraded);
    EXPECT_EQ(trafficSpy.count(), 1);
}

TEST_F(PollingControllerTest, DegradedState_DifferentiatingErrorsReportImmediately) {
    QSignalSpy trafficSpy(controller_.get(), &PollingController::trafficEvent);

    controller_->handleSessionConnected();
    const auto spec = makePollSpec();

    // 1st failure: crc error enters Degraded
    controller_->startPolling(spec);
    controller_->handleResponse(false, 0, 0, QStringLiteral("crc error"));
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Degraded);
    EXPECT_EQ(trafficSpy.count(), 1);

    // 2nd failure with a different error text should bypass rate limiter and report immediately
    controller_->onPollTimerTick();
    controller_->handleResponse(false, 0, 0, QStringLiteral("parity error"));
    processStateMachine();

    EXPECT_EQ(controller_->currentState(), PollState::Degraded);
    EXPECT_EQ(trafficSpy.count(), 2);
}

TEST_F(PollingControllerTest, IsSuppressingTrafficLog_RemainsTrueBetweenPollTicks) {
    controller_->handleSessionConnected();
    const auto spec = makePollSpec();

    EXPECT_FALSE(controller_->isSuppressingTrafficLog());

    controller_->startPolling(spec);
    processStateMachine();
    EXPECT_TRUE(controller_->isPollingActive());
    EXPECT_TRUE(controller_->isSuppressingTrafficLog());

    // After tick completes, polling stays active between timer ticks.
    controller_->handleResponse(true, 5, 0, QString());
    processStateMachine();
    EXPECT_FALSE(controller_->context().requestInFlight);
    EXPECT_TRUE(controller_->isPollingActive());
    EXPECT_TRUE(controller_->isSuppressingTrafficLog());

    controller_->stopPoll();
    processStateMachine();
    EXPECT_FALSE(controller_->isPollingActive());
    EXPECT_FALSE(controller_->isSuppressingTrafficLog());
}

TEST_F(PollingControllerTest, CustomIntervalPreservedOnStartPolling) {
    controller_->handleSessionConnected();
    auto spec = makePollSpec(250);
    controller_->startPolling(spec);

    auto* timer = controller_->findChild<QTimer*>();
    ASSERT_NE(timer, nullptr);
    EXPECT_EQ(timer->interval(), 250);
    controller_->stopPoll();
}

TEST_F(PollingControllerTest, LateResponseAfterStopPoll_DiscardedWithoutCrash) {
    controller_->handleSessionConnected();
    controller_->startPolling(makePollSpec());
    processStateMachine();
    EXPECT_EQ(controller_->currentState(), PollState::Polling);

    controller_->stopPoll();
    processStateMachine();
    EXPECT_EQ(controller_->currentState(), PollState::Idle);

    // Late failure arrives after stopPoll(): must NOT crash with Idle -> Escalated assertion failure
    EXPECT_NO_THROW({
        controller_->handleResponse(false, 0, 0, ::modbus::base::ModbusError::Timeout,
                                    QStringLiteral("Late Timeout"));
    });
    EXPECT_EQ(controller_->currentState(), PollState::Idle);
}

TEST_F(PollingControllerTest, ErrorClassification_TypedI18nChinese) {
    controller_->handleSessionConnected();
    // 1000ms interval -> base threshold = 3.
    // Timeout error severity tightens threshold to 2.
    auto spec = makePollSpec(1000);
    controller_->startPolling(spec);

    // 1st timeout with Chinese localization: degrades
    controller_->handleResponse(false, 0, 0, ::modbus::base::ModbusError::Timeout,
                                QStringLiteral("请求超时"));
    EXPECT_EQ(controller_->currentState(), PollState::Degraded);

    // 2nd timeout with Chinese localization: escalates (consecutiveErrors 2 >= effectiveThreshold 2)
    controller_->onPollTimerTick();
    controller_->handleResponse(false, 0, 0, ::modbus::base::ModbusError::Timeout,
                                QStringLiteral("请求超时"));
    EXPECT_EQ(controller_->currentState(), PollState::Escalated);
}

TEST_F(PollingControllerTest, WritePolling_BuildsWritePduAndSubmits) {
    QSignalSpy spy(controller_.get(), &PollingController::submitPollRequest);

    controller_->handleSessionConnected();
    PollSpec spec;
    spec.functionCode = 0x06;
    spec.startAddress = 10;
    spec.quantity = 1;
    spec.slaveId = 1;
    spec.intervalMs = 500;
    spec.writeData = QByteArray::fromHex("002A");

    controller_->startPolling(spec);
    processStateMachine();

    ASSERT_EQ(spy.count(), 1);
    const auto pdu = spy.first().at(0).value<::modbus::base::Pdu>();
    EXPECT_EQ(pdu.functionCode(), ::modbus::base::FunctionCode::WriteSingleRegister);
    controller_->stopPoll();
}

} // namespace
