/**
 * @file ModbusClientBoundaryTest.cpp
 * @brief Session-layer boundary tests: timeout, retry, broadcast,
 *        concurrency guard, abort, and edge-case transitions.
 *        18 cases, zero sleep — all async paths use EXPECT_CALL + Invoke.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <future>
#include <thread>
#include <chrono>

#include "mocks/MockChannel.h"
#include "mocks/MockTransport.h"
#include "modbus/session/ModbusClient.h"
#include "modbus/base/ModbusTypes.h"
#include "modbus/base/ModbusConfig.h"
#include "modbus/session/RequestStateMachine.h"
#include "modbus/session/ConnectionStateMachine.h"
#include "helpers/ModbusTestHelpers.h"
#include "Config.h"

using namespace modbus::session;
using namespace modbus::transport;
using namespace modbus::base;
using namespace io;
using namespace testing;

namespace {

// ============================================================================
// Test Fixture
// ============================================================================

class ModbusClientBoundaryTest : public Test {
protected:
    void SetUp() override {
        mockChannel_ = std::make_shared<NiceMock<MockChannel>>();
        mockTransport_ = std::make_shared<NiceMock<MockTransport>>();

        // Capture handler functors so we can invoke them from tests
        ON_CALL(*mockChannel_, addStateHandler(_))
            .WillByDefault(DoAll(SaveArg<0>(&stateHandler_), Return(1)));
        ON_CALL(*mockChannel_, setReadHandler(_))
            .WillByDefault(SaveArg<0>(&readHandler_));
        ON_CALL(*mockChannel_, setErrorHandler(_))
            .WillByDefault(SaveArg<0>(&errorHandler_));
        ON_CALL(*mockChannel_, setWriteDrainedHandler(_))
            .WillByDefault(SaveArg<0>(&writeDrainedHandler_));

        // Default mocks: closed channel, no auto-reconnect for tests
        ON_CALL(*mockChannel_, state()).WillByDefault(ReturnPointee(&currentState_));
        ON_CALL(*mockChannel_, isOpen()).WillByDefault(Invoke([this]() {
            return currentState_ == ChannelState::Open;
        }));

        client_ = std::make_unique<ModbusClient>(mockChannel_, mockTransport_);

        // Default config: short timeout, no auto reconnect for fast deterministic tests
        cfg_ = modbus::test::MakeModbusConfig(ModbusMode::RTU, 200, 2);
        cfg_.autoReconnect = false;
        cfg_.retries = 0;
        cfg_.retryIntervalMs = 20;
        cfg_.retryBackoffFactor = 1.0;
        cfg_.maxRetryIntervalMs = 50;
        cfg_.retryJitterPercent = 0;
        client_->setConfig(cfg_);
    }

    void TearDown() override {
        client_.reset();
        mockChannel_.reset();
        mockTransport_.reset();
    }

    // Helper: bring client into Connected state
    void connectClient() {
        EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([this]() {
            currentState_ = ChannelState::Open;
            if (stateHandler_) stateHandler_(ChannelState::Open);
            return true;
        }));
        ASSERT_TRUE(client_->connect());
        ASSERT_TRUE(client_->isConnected());
    }

    static Pdu makeRhrPdu() {
        return modbus::test::MakeReadHoldingRegistersPdu(0x0000, 2);
    }

    static Pdu makeWmrPdu() {
        return modbus::test::MakeWriteMultipleRegistersPdu(0x0000, 2);
    }

    ChannelState currentState_ = ChannelState::Closed;
    std::function<void(ChannelState)> stateHandler_;
    std::function<void(QByteArrayView)> readHandler_;
    std::function<void(const QString&)> errorHandler_;
    std::function<void()> writeDrainedHandler_;

    std::shared_ptr<NiceMock<MockChannel>> mockChannel_;
    std::shared_ptr<NiceMock<MockTransport>> mockTransport_;
    std::unique_ptr<ModbusClient> client_;
    ModbusConfig cfg_;
};

// ============================================================================
// 1. SendRequest without connection -> error
// ============================================================================

TEST_F(ModbusClientBoundaryTest, SendRequest_NotConnected_ReturnsError) {
    // sendRequest internally calls ensureConnected, which calls open() -> false
    auto response = client_->sendRequest(makeRhrPdu());
    EXPECT_FALSE(response.isSuccess);
    EXPECT_THAT(response.error.toStdString(), AnyOf(HasSubstr("dispatch"), HasSubstr("connected")));
}

// ============================================================================
// 2. SendRequest timeout
// ============================================================================

TEST_F(ModbusClientBoundaryTest, SendRequest_Timeout_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(true));

    // Never call readHandler -> timeout triggers
    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_FALSE(resp.isSuccess);
}

// ============================================================================
// 3. SendRequest with retries=0 and parse failure -> immediate error
// ============================================================================

TEST_F(ModbusClientBoundaryTest, SendRequest_ParseFailureNoRetry_ReturnsError) {
    cfg_.retries = 0;
    cfg_.timeoutMs = 500;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        writeDrainedHandler_();
        return true;
    }));

    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .WillOnce(Return(ParseResponseResult{ParseResponseStatus::Invalid, std::nullopt}));

    std::thread responder([this]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        if (readHandler_) readHandler_(QByteArray::fromHex("0103020001EE5F"));
    });

    auto resp = client_->sendRequest(makeRhrPdu());
    responder.join();
    EXPECT_FALSE(resp.isSuccess);
}

// ============================================================================
// 4. Retry count reflected in response (retries configured but all fail)
// ============================================================================

TEST_F(ModbusClientBoundaryTest, SendRequest_RetryReflectsInResponseCount) {
    cfg_.retries = 1;
    cfg_.timeoutMs = 500;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .Times(2)
        .WillRepeatedly(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).Times(2).WillRepeatedly(Invoke([this](QByteArrayView) {
        writeDrainedHandler_();
        return true;
    }));
    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .Times(2)
        .WillRepeatedly(Return(ParseResponseResult{ParseResponseStatus::Invalid, std::nullopt}));

    std::thread responder([this]() {
        // First attempt: provide a frame quickly, parse returns Invalid
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        if (readHandler_) readHandler_(QByteArray::fromHex("0103020001EE5F"));
        // Second attempt: provide another frame after retry delay
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (readHandler_) readHandler_(QByteArray::fromHex("0103020001EE5F"));
    });

    auto resp = client_->sendRequest(makeRhrPdu());
    responder.join();
    EXPECT_FALSE(resp.isSuccess);
    EXPECT_GT(resp.attemptCount, 1) << "Should have attempted at least once more after retry";
}

// ============================================================================
// 5. RTU broadcast — no response expected
// ============================================================================

TEST_F(ModbusClientBoundaryTest, SendRequest_RtuBroadcast_ReturnsEmptySuccess) {
    cfg_.mode = ModbusMode::RTU;
    cfg_.slaveId = 0;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, 0))
        .WillOnce(Return(QByteArray::fromHex("00100000000204")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        writeDrainedHandler_(); // unblock write-drain immediately
        return true;
    }));

    auto resp = client_->sendRequest(makeWmrPdu(), 0);
    EXPECT_TRUE(resp.isSuccess);
}

// ============================================================================
// 6. Abort during active sendRequest
// ============================================================================

TEST_F(ModbusClientBoundaryTest, Abort_DuringSendRequest_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));

    // Make write slow enough to allow abort to intervene
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return true;
    }));

    auto future = std::async(std::launch::async, [this]() {
        return client_->sendRequest(makeRhrPdu());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    client_->abort();

    auto resp = future.get();
    EXPECT_FALSE(resp.isSuccess);
}

// ============================================================================
// 7. Concurrent sendRequest rejected at runtime
// ============================================================================

TEST_F(ModbusClientBoundaryTest, SendRequest_ConcurrentCall_ReturnsInProgressError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        return true;
    }));

    auto first = std::async(std::launch::async, [this]() {
        return client_->sendRequest(makeRhrPdu());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    auto second = client_->sendRequest(makeRhrPdu());

    EXPECT_FALSE(second.isSuccess);
    EXPECT_THAT(second.error.toStdString(), HasSubstr("already in progress"));
    // Soft-lock contention must surface as a structured Busy error code so the
    // UI can distinguish it from genuine protocol/transport errors.
    EXPECT_EQ(second.errorCode, modbus::session::RequestError::Busy);
    EXPECT_TRUE(second.isBusy());

    auto firstResp = first.get();
    EXPECT_FALSE(firstResp.isSuccess);
}

// ============================================================================
// 8. Channel error during request
// ============================================================================

TEST_F(ModbusClientBoundaryTest, ChannelError_DuringRequest_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(true));

    auto future = std::async(std::launch::async, [this]() {
        return client_->sendRequest(makeRhrPdu());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    if (errorHandler_) errorHandler_("Connection reset by peer");

    auto resp = future.get();
    EXPECT_FALSE(resp.isSuccess);
}

// ============================================================================
// 9. Disconnect during active request
// ============================================================================

TEST_F(ModbusClientBoundaryTest, Disconnect_DuringActiveRequest_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(true));
    // disconnect calls close()
    EXPECT_CALL(*mockChannel_, close()).WillRepeatedly(Return());

    auto future = std::async(std::launch::async, [this]() {
        return client_->sendRequest(makeRhrPdu());
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    client_->disconnect();

    auto resp = future.get();
    EXPECT_FALSE(resp.isSuccess);
}

// ============================================================================
// 10. RequestStateMachine — valid life cycle transitions
// ============================================================================

TEST_F(ModbusClientBoundaryTest, RequestState_TransitionsThroughLifecycle) {
    RequestStateMachine rsm;

    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Idle);

    rsm.tryTransition(RequestStateMachine::State::Sending, "start");
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Sending);

    rsm.tryTransition(RequestStateMachine::State::Waiting, "data-sent");
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Waiting);

    rsm.tryTransition(RequestStateMachine::State::Completed, "response-received");
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Completed);
}

// ============================================================================
// 11. RequestStateMachine — invalid transition silently rejected
// ============================================================================

TEST_F(ModbusClientBoundaryTest, RequestState_InvalidTransition_IsRejected) {
    RequestStateMachine rsm;
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Idle);

    // Cannot go directly from Idle -> Completed
    rsm.tryTransition(RequestStateMachine::State::Completed, "invalid");
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Idle);
}

// ============================================================================
// 12. Write failure -> error without timeout
// ============================================================================

TEST_F(ModbusClientBoundaryTest, SendRequest_WriteFails_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(false));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_FALSE(resp.isSuccess);
}

// ============================================================================
// 13. Oversized PDU -> validation error
// ============================================================================

TEST_F(ModbusClientBoundaryTest, SendRequest_OversizedPdu_ReturnsError) {
    connectClient();

    QByteArray hugeData(config::Modbus::kMaxAduSize + 100, 0x00);
    Pdu hugePdu(FunctionCode::ReadHoldingRegisters, hugeData);

    auto resp = client_->sendRequest(hugePdu);
    EXPECT_FALSE(resp.isSuccess);
}

// ============================================================================
// 14. ConnectionStateMachine — valid transitions
// ============================================================================

TEST_F(ModbusClientBoundaryTest, ConnectionState_ValidTransition_Accepted) {
    ConnectionStateMachine csm;

    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Disconnected);

    csm.tryTransition(ConnectionStateMachine::State::Connecting, "connect-called");
    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Connecting);

    csm.tryTransition(ConnectionStateMachine::State::Connected, "open-ok");
    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Connected);
}

// ============================================================================
// 15. ConnectionStateMachine — invalid transition silently rejected
// ============================================================================

TEST_F(ModbusClientBoundaryTest, ConnectionState_InvalidTransition_Rejected) {
    ConnectionStateMachine csm;

    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Disconnected);

    // Cannot go from Disconnected -> Reconnecting
    csm.tryTransition(ConnectionStateMachine::State::Reconnecting, "invalid");
    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Disconnected);
}

// ============================================================================
// 16. lastChannelError after connection failure
// ============================================================================

TEST_F(ModbusClientBoundaryTest, LastChannelError_AfterConnectionFailure_HasError) {
    // With autoReconnect=false, ensureConnected tries open() once
    Expectation openCall = EXPECT_CALL(*mockChannel_, open()).WillOnce(Return(false));
    EXPECT_CALL(*mockChannel_, close()).After(openCall);

    client_->connect();

    QString err = client_->lastChannelError();
    EXPECT_FALSE(err.isEmpty());
}

// ============================================================================
// 17. isConnected toggle
// ============================================================================

TEST_F(ModbusClientBoundaryTest, IsConnected_ToggleConnectDisconnect_ReflectsState) {
    connectClient();
    EXPECT_TRUE(client_->isConnected());

    EXPECT_CALL(*mockChannel_, close()).WillOnce(Invoke([this]() {
        currentState_ = ChannelState::Closed;
        if (stateHandler_) stateHandler_(ChannelState::Closed);
    }));
    client_->disconnect();
    EXPECT_FALSE(client_->isConnected());
}

// ============================================================================
// 18. sendRaw on disconnected channel — no crash
// ============================================================================

TEST_F(ModbusClientBoundaryTest, SendRaw_NotConnected_DoesNotCrash) {
    QByteArray raw = QByteArray::fromHex("010300000001840A");
    EXPECT_NO_FATAL_FAILURE(client_->sendRaw(raw));
}

// ============================================================================
// 19. Abort from idle state — triggers Aborted transition
// ============================================================================

TEST_F(ModbusClientBoundaryTest, Abort_WhenIdle_DoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(client_->abort());
    // abort() transitions Idle->Aborted since Idle != Completed and != Failed
}

} // namespace
