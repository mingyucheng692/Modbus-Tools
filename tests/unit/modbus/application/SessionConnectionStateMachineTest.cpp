#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSignalSpy>

#include "../../../ui/application/modbus/SessionConnectionStateMachine.h"

using namespace ui::application::modbus;

namespace {

class SessionConnectionStateMachineTest : public ::testing::Test {
protected:
    void SetUp() override {
        machine_ = std::make_unique<SessionConnectionStateMachine>();
    }

    std::unique_ptr<SessionConnectionStateMachine> machine_;
};

TEST_F(SessionConnectionStateMachineTest, InitialStateIsDisconnected) {
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Disconnected);
}

TEST_F(SessionConnectionStateMachineTest, LegalTransition_EmitsStateChanged) {
    QSignalSpy spy(machine_.get(), &SessionConnectionStateMachine::stateChanged);

    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Connecting);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().value(0).value<SessionConnectionState>(),
              SessionConnectionState::Connecting);
}

TEST_F(SessionConnectionStateMachineTest, IdempotentTransition_SucceedsNoSignal) {
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    QSignalSpy spy(machine_.get(), &SessionConnectionStateMachine::stateChanged);

    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Connecting);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(SessionConnectionStateMachineTest, IllegalTransition_RejectedNoStateChange) {
    // Disconnected -> Connected is illegal (must go through Connecting).
    QSignalSpy spy(machine_.get(), &SessionConnectionStateMachine::stateChanged);

    EXPECT_FALSE(machine_->transitionTo(SessionConnectionState::Connected));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Disconnected);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(SessionConnectionStateMachineTest, IllegalTransition_FromDisconnecting_Rejected) {
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Disconnecting));

    EXPECT_FALSE(machine_->transitionTo(SessionConnectionState::Connected));
    EXPECT_FALSE(machine_->transitionTo(SessionConnectionState::Connecting));
    EXPECT_FALSE(machine_->transitionTo(SessionConnectionState::TransportConnected));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Disconnecting);
}

TEST_F(SessionConnectionStateMachineTest, IllegalTransition_FromTransportConnected_Rejected) {
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::TransportConnected));

    // Cannot reconnect without tearing down first.
    EXPECT_FALSE(machine_->transitionTo(SessionConnectionState::Connecting));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::TransportConnected);
}

TEST_F(SessionConnectionStateMachineTest, FullHappyPath_TcpConnect) {
    // Disconnected -> Connecting -> TransportConnected -> Connected
    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::TransportConnected));
    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Connected));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Connected);
}

TEST_F(SessionConnectionStateMachineTest, ConnectFailed_BackToDisconnected) {
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Disconnected));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Disconnected);
}

TEST_F(SessionConnectionStateMachineTest, UserDisconnect_ConnectedToDisconnectingToDisconnected) {
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connected));

    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Disconnecting));
    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Disconnected));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Disconnected);
}

TEST_F(SessionConnectionStateMachineTest, SpuriousOpenWhileConnected_Tolerated) {
    // The original code tolerated a channel-Open event while already Connected;
    // the Connected -> TransportConnected transition must remain legal.
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connected));

    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::TransportConnected));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::TransportConnected);

    // And recovery back to Connected must be legal.
    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Connected));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Connected);
}

TEST_F(SessionConnectionStateMachineTest, TransportConnectedDirectlyToConnected) {
    // RTU fast path: Connecting -> Connected (no explicit TransportConnected).
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Connected));
    EXPECT_EQ(machine_->currentState(), SessionConnectionState::Connected);
}

TEST_F(SessionConnectionStateMachineTest, DisconnectWhileConnecting) {
    ASSERT_TRUE(machine_->transitionTo(SessionConnectionState::Connecting));
    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Disconnecting));
    EXPECT_TRUE(machine_->transitionTo(SessionConnectionState::Disconnected));
}

TEST_F(SessionConnectionStateMachineTest, IsLegalTransition_Table) {
    using S = SessionConnectionState;

    // Self-transitions always legal (idempotent re-entry).
    for (int i = 0; i < 5; ++i) {
        const auto s = static_cast<S>(i);
        EXPECT_TRUE(SessionConnectionStateMachine::isLegalTransition(s, s))
            << "self-transition for state " << i;
    }

    // Representative legal cross-transitions.
    EXPECT_TRUE(SessionConnectionStateMachine::isLegalTransition(S::Disconnected, S::Connecting));
    EXPECT_TRUE(SessionConnectionStateMachine::isLegalTransition(S::Connecting, S::TransportConnected));
    EXPECT_TRUE(SessionConnectionStateMachine::isLegalTransition(S::Connecting, S::Connected));
    EXPECT_TRUE(SessionConnectionStateMachine::isLegalTransition(S::TransportConnected, S::Connected));
    EXPECT_TRUE(SessionConnectionStateMachine::isLegalTransition(S::Connected, S::Disconnecting));
    EXPECT_TRUE(SessionConnectionStateMachine::isLegalTransition(S::Disconnecting, S::Disconnected));

    // Representative illegal cross-transitions.
    EXPECT_FALSE(SessionConnectionStateMachine::isLegalTransition(S::Disconnected, S::Connected));
    EXPECT_FALSE(SessionConnectionStateMachine::isLegalTransition(S::Disconnected, S::TransportConnected));
    EXPECT_FALSE(SessionConnectionStateMachine::isLegalTransition(S::Disconnected, S::Disconnecting));
    EXPECT_FALSE(SessionConnectionStateMachine::isLegalTransition(S::Connected, S::Connecting));
    EXPECT_FALSE(SessionConnectionStateMachine::isLegalTransition(S::Disconnecting, S::Connecting));
    EXPECT_FALSE(SessionConnectionStateMachine::isLegalTransition(S::Disconnecting, S::Connected));
    EXPECT_FALSE(SessionConnectionStateMachine::isLegalTransition(S::TransportConnected, S::Connecting));
}

} // namespace
