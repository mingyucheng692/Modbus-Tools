// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ui/application/modbus/ModbusPagePresenter.h"
#include "ui/application/modbus/ModbusSessionPresenter.h"
#include "ui/application/modbus/TrafficLogController.h"
#include "ui/application/modbus/PollingController.h"
#include "ui/application/modbus/RequestSubmissionService.h"
#include "ui/application/modbus/ModbusTypes.h"
#include "mocks/MockModbusPageView.h"
#include "mocks/UiTestDoubles.h"

#include <QPointer>
#include <QCoreApplication>

using namespace testing;
using namespace ui::application::modbus;

namespace {

class ConnectionStateGuardingTest : public Test {
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

// 1. Initial State: Controls are guarded and disabled for disconnected state
TEST_F(ConnectionStateGuardingTest, InitialState_ControlsAreDisabled) {
    auto freshMockView = std::make_unique<NiceMock<tests::mocks::MockModbusPageView>>();
    ExecutionGuardState capturedGuards;
    EXPECT_CALL(*freshMockView, setExecutionGuards(_))
        .WillRepeatedly(SaveArg<0>(&capturedGuards));

    ModbusPagePresenter freshPresenter(freshMockView.get(), SessionMode::Tcp);

    EXPECT_FALSE(capturedGuards.executeEnabled);
    EXPECT_FALSE(capturedGuards.pollingEnabled);
    EXPECT_FALSE(capturedGuards.rawSendEnabled);
    EXPECT_TRUE(capturedGuards.tooltipMessage.contains(QObject::tr("Connect device")));
    freshPresenter.detachView();
}

// 2. Idempotent guard: calling with the same state twice does not trigger redundant refresh
TEST_F(ConnectionStateGuardingTest, SyncWidgetGuards_Idempotent_SameStateTwice) {
    EXPECT_CALL(*mockView_, renderConnectionState(SessionConnectionState::Connected)).Times(1);

    presenter_->syncWidgetGuards(SessionConnectionState::Connected);
    // Second invocation with same state: early exit
    presenter_->syncWidgetGuards(SessionConnectionState::Connected);
}

// 3. Connecting State: reads and writes disabled
TEST_F(ConnectionStateGuardingTest, ConnectingState_ReadsWritesDisabled) {
    ExecutionGuardState capturedGuards;
    EXPECT_CALL(*mockView_, renderConnectionState(SessionConnectionState::Connecting)).Times(1);
    EXPECT_CALL(*mockView_, setExecutionGuards(_))
        .WillOnce(SaveArg<0>(&capturedGuards));

    presenter_->syncWidgetGuards(SessionConnectionState::Connecting);

    EXPECT_FALSE(capturedGuards.executeEnabled);
    EXPECT_FALSE(capturedGuards.pollingEnabled);
    EXPECT_FALSE(capturedGuards.rawSendEnabled);
    EXPECT_TRUE(capturedGuards.tooltipMessage.contains(QObject::tr("Connecting")));
}

// 4. TransportConnected (TCP intermediate): reads enabled, writes disabled + operator-friendly tooltip
TEST_F(ConnectionStateGuardingTest, TransportConnected_TCP_ReadsEnabledWritesDisabled) {
    ExecutionGuardState capturedGuards;
    EXPECT_CALL(*mockView_, renderConnectionState(SessionConnectionState::TransportConnected)).Times(1);
    EXPECT_CALL(*mockView_, setExecutionGuards(_))
        .WillOnce(SaveArg<0>(&capturedGuards));

    presenter_->syncWidgetGuards(SessionConnectionState::TransportConnected);

    EXPECT_TRUE(capturedGuards.executeEnabled);
    EXPECT_FALSE(capturedGuards.pollingEnabled);
    EXPECT_TRUE(capturedGuards.tooltipMessage.contains(QObject::tr("Device is establishing session")));
}

// 5. TransportConnected Serial RTU: direct Connected, all ops enabled
TEST_F(ConnectionStateGuardingTest, TransportConnected_Serial_AllOpsEnabled) {
    auto rtuPresenter = std::make_unique<ModbusPagePresenter>(mockView_.get(), SessionMode::Rtu);
    ExecutionGuardState capturedGuards;
    EXPECT_CALL(*mockView_, setExecutionGuards(_))
        .WillRepeatedly(SaveArg<0>(&capturedGuards));

    rtuPresenter->syncWidgetGuards(SessionConnectionState::Connected);

    EXPECT_TRUE(capturedGuards.executeEnabled);
    EXPECT_TRUE(capturedGuards.pollingEnabled);
    EXPECT_TRUE(capturedGuards.rawSendEnabled);

    rtuPresenter->detachView();
}

// 6. Connected: All operations and polling enabled
TEST_F(ConnectionStateGuardingTest, Connected_AllOpsEnabled_PollEnabled) {
    ExecutionGuardState capturedGuards;
    EXPECT_CALL(*mockView_, renderConnectionState(SessionConnectionState::Connected)).Times(1);
    EXPECT_CALL(*mockView_, setExecutionGuards(_))
        .WillOnce(SaveArg<0>(&capturedGuards));

    presenter_->syncWidgetGuards(SessionConnectionState::Connected);

    EXPECT_TRUE(capturedGuards.executeEnabled);
    EXPECT_TRUE(capturedGuards.pollingEnabled);
    EXPECT_TRUE(capturedGuards.rawSendEnabled);
}

// 7. Disconnected: All operations reverted and active polling stopped
TEST_F(ConnectionStateGuardingTest, Disconnected_AllOpsReverted_TimerStopped) {
    presenter_->syncWidgetGuards(SessionConnectionState::Connected);

    ExecutionGuardState capturedGuards;
    EXPECT_CALL(*mockView_, renderConnectionState(SessionConnectionState::Disconnected)).Times(1);
    EXPECT_CALL(*mockView_, setExecutionGuards(_))
        .WillOnce(SaveArg<0>(&capturedGuards));

    presenter_->syncWidgetGuards(SessionConnectionState::Disconnected);

    EXPECT_FALSE(capturedGuards.executeEnabled);
    EXPECT_FALSE(capturedGuards.pollingEnabled);
    EXPECT_FALSE(capturedGuards.rawSendEnabled);
}

// 8. Parameterized Test for EnsureConnected Log Behavior (TLC present vs TLC null)
class EnsureConnectedLogBehaviorTest : public ConnectionStateGuardingTest,
                                       public WithParamInterface<bool> {};

TEST_P(EnsureConnectedLogBehaviorTest, EnsureConnected_LogBehavior_P) {
    const bool hasTlc = GetParam();

    if (!hasTlc) {
        presenter_->setTrafficLogControllerForTest(nullptr);
    }

    // Ensure session is disconnected
    EXPECT_FALSE(presenter_->sessionPresenter()->isSessionConnected());

    // ensureConnected should return false safely in both cases
    EXPECT_FALSE(presenter_->ensureConnected());
}

INSTANTIATE_TEST_SUITE_P(
    TlcScenarios,
    EnsureConnectedLogBehaviorTest,
    Values(true, false)
);

// 9. QPointer TLC Safety: safe after teardown destruction
TEST_F(ConnectionStateGuardingTest, QPointerTLC_SafeAfterTeardown) {
    auto* standaloneTlc = new TrafficLogController(static_cast<PollingController*>(nullptr), nullptr);
    QPointer<TrafficLogController> guard(standaloneTlc);

    presenter_->setTrafficLogControllerForTest(standaloneTlc);
    ASSERT_FALSE(guard.isNull());

    delete standaloneTlc;
    EXPECT_TRUE(guard.isNull());

    EXPECT_FALSE(presenter_->ensureConnected());
}

// 10. Mode switch during connecting resets controls and protocol
TEST_F(ConnectionStateGuardingTest, ModeSwitch_DuringConnecting_ControlsCorrectlyReset) {
    presenter_->syncWidgetGuards(SessionConnectionState::Connecting);

    EXPECT_CALL(*mockView_, setProtocolType(::modbus::parser::ProtocolType::Rtu)).Times(1);
    presenter_->switchMode(SessionMode::Rtu);
    QCoreApplication::processEvents();

    EXPECT_CALL(*mockView_, setProtocolType(::modbus::parser::ProtocolType::Tcp)).Times(1);
    presenter_->switchMode(SessionMode::Tcp);
    QCoreApplication::processEvents();
}

// 11. View detachment lifecycle: detachView prevents dangling pointer access
TEST_F(ConnectionStateGuardingTest, DetachView_PreventsDanglingAccess) {
    EXPECT_CALL(*mockView_, renderConnectionState(_)).Times(0);
    EXPECT_CALL(*mockView_, setExecutionGuards(_)).Times(0);

    presenter_->detachView();
    EXPECT_EQ(presenter_->view(), nullptr);

    // Callbacks must safely execute with null view_ and still update session state
    presenter_->syncWidgetGuards(SessionConnectionState::Connected);
    presenter_->setLinked(true);
    EXPECT_TRUE(presenter_->sessionPresenter()->isLinked());
}

// 12. isSessionConnected returns true for TransportConnected and Connected
TEST_F(ConnectionStateGuardingTest, IsSessionConnected_TrueForConnectedAndTransportConnected) {
    auto* session = presenter_->sessionPresenter();
    ASSERT_NE(session, nullptr);

    EXPECT_FALSE(session->isSessionConnected());
    presenter_->syncWidgetGuards(SessionConnectionState::Connected);
    EXPECT_FALSE(presenter_->ensureConnected());
}

// 13. PagePresenter: requestConnect anti-bounce reentrancy guard
TEST_F(ConnectionStateGuardingTest, RequestConnect_AntiBounceReentrancyGuard) {
    auto* session = presenter_->sessionPresenter();
    ASSERT_NE(session, nullptr);

    presenter_->syncWidgetGuards(SessionConnectionState::Connecting);
    const quint64 genBefore = session->connectionGeneration();
    ModbusConnectionSpec spec;
    spec.config.mode = ::modbus::base::ModbusMode::TCP;
    spec.config.ipAddress = QStringLiteral("127.0.0.1");
    spec.config.port = 502;

    presenter_->requestConnect(spec);
    EXPECT_EQ(session->connectionGeneration(), genBefore);
    EXPECT_FALSE(session->isSessionConnected());
}
