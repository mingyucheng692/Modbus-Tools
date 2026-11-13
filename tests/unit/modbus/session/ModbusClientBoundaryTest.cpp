// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <QTimer>
#include <QtTest>

#include "mocks/MockChannel.h"
#include "mocks/MockTransport.h"
#include "modbus/session/ModbusClient.h"
#include "modbus/base/ModbusTypes.h"
#include "modbus/base/ModbusConfig.h"
#include "modbus/base/ModbusCrc.h"
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

// Builds a minimal RTU response ADU (slave 01) so fed bytes survive the
// extractor's framing: the mock transport ignores the frame content, but the
// FrameExtractor must see a plausibly shaped ADU before parseResponse runs.
// Delivered via QMetaObject::invokeMethod on the main thread — the read
// handler is a channel-IO-thread contract and extractor state must not be
// touched concurrently with checkPendingResponse().
QByteArray rtuFrame(uint8_t functionCode, const QByteArray& data) {
    QByteArray frame;
    frame.append(static_cast<char>(0x01)); // slave
    frame.append(static_cast<char>(functionCode));
    frame.append(data);
    const uint16_t crc = calculateModbusRtuCrc(QByteArrayView(frame));
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));
    return frame;
}

// Test Fixture
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

        // Default mocks: closed channel for tests
        ON_CALL(*mockChannel_, state()).WillByDefault(ReturnPointee(&currentState_));
        ON_CALL(*mockChannel_, isOpen()).WillByDefault(Invoke([this]() {
            return currentState_ == ChannelState::Open;
        }));

        client_ = std::make_unique<ModbusClient>(mockChannel_, mockTransport_);

        // Default config: short timeout for fast deterministic tests
        cfg_ = modbus::test::MakeModbusConfig(ModbusMode::RTU, 200, 2);
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

    // Delivers a response frame on the main-thread event loop after a delay.
    // The synchronous request wait pumps that loop, so deliveries fire on
    // schedule and stay on the thread that owns the extractor state — a
    // sleeping worker thread marshalled through queued invocations instead
    // stacks scheduler latency on every delivery, which breaks the retry
    // and timeout budgets on loaded CI runners.
    void feedFrameAfter(int delayMs, const QByteArray& frame) {
        QTimer::singleShot(delayMs, QCoreApplication::instance(), [this, frame]() {
            if (readHandler_) {
                readHandler_(frame);
            }
        });
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
    std::function<void(const io::ChannelError&)> errorHandler_;
    std::function<void()> writeDrainedHandler_;

    std::shared_ptr<NiceMock<MockChannel>> mockChannel_;
    std::shared_ptr<NiceMock<MockTransport>> mockTransport_;
    std::unique_ptr<ModbusClient> client_;
    ModbusConfig cfg_;
};

// 1. SendRequest without connection -> error
TEST_F(ModbusClientBoundaryTest, SendRequest_NotConnected_ReturnsError) {
    // sendRequest internally calls ensureConnected, which calls open() -> false
    auto response = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(response.isError());
    EXPECT_THAT(response.error.toStdString(), AnyOf(HasSubstr("dispatch"), HasSubstr("connected")));
}

// 2. SendRequest timeout
TEST_F(ModbusClientBoundaryTest, SendRequest_Timeout_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(true));

    // Never call readHandler -> timeout triggers
    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
}

// 3. SendRequest with retries=0 and parse failure -> immediate error
TEST_F(ModbusClientBoundaryTest, SendRequest_ParseFailureNoRetry_ReturnsError) {
    cfg_.retries = 0;
    cfg_.timeoutMs = 500;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        writeDrainedHandler_();
        feedFrameAfter(0, QByteArray::fromHex("0103020001EE5F"));
        return true;
    }));

    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .WillOnce(Return(ParseResponseResult{ParseResponseStatus::Invalid, std::nullopt}));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
}

// 4. Retry count reflected in response (retries configured but all fail)
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
        feedFrameAfter(0, QByteArray::fromHex("0103020001EE5F"));
        return true;
    }));
    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .Times(2)
        .WillRepeatedly(Return(ParseResponseResult{ParseResponseStatus::Invalid, std::nullopt}));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
    EXPECT_GT(resp.attemptCount, 1) << "Should have attempted at least once more after retry";
}

// 5. RTU broadcast — no response expected
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
    EXPECT_FALSE(resp.isError());
}

// 6. Abort during active sendRequest
TEST_F(ModbusClientBoundaryTest, Abort_DuringSendRequest_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        QTimer::singleShot(0, QCoreApplication::instance(), [this]() {
            client_->abort();
        });
        return true;
    }));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
}

// 7. Concurrent sendRequest rejected at runtime
TEST_F(ModbusClientBoundaryTest, SendRequest_ConcurrentCall_ReturnsInProgressError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(true));

    bool firstCompleted = false;
    ModbusResponse firstResp;
    client_->sendRequestAsync(makeRhrPdu(), 1, 0, [&](const ModbusResponse& response) {
        firstCompleted = true;
        firstResp = response;
    });

    auto second = client_->sendRequest(makeRhrPdu());

    EXPECT_TRUE(second.isError());
    EXPECT_THAT(second.error.toStdString(), HasSubstr("already in progress"));
    // Soft-lock contention must surface as a structured Busy error code so the
    // UI can distinguish it from genuine protocol/transport errors.
    EXPECT_EQ(second.errorCode, modbus::base::ModbusError::Busy);
    EXPECT_TRUE(second.isBusy());

    client_->abort();
    EXPECT_TRUE(firstCompleted);
    EXPECT_TRUE(firstResp.isError());
}

// 8. Channel error during request
TEST_F(ModbusClientBoundaryTest, ChannelError_DuringRequest_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        QTimer::singleShot(0, QCoreApplication::instance(), [this]() {
            if (errorHandler_) {
                errorHandler_(io::ChannelError{io::ChannelErrorCode::ConnectionReset,
                                              QStringLiteral("Connection reset by peer")});
            }
        });
        return true;
    }));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
}

// 9. Disconnect during active request
TEST_F(ModbusClientBoundaryTest, Disconnect_DuringActiveRequest_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        QTimer::singleShot(0, QCoreApplication::instance(), [this]() {
            client_->disconnect();
        });
        return true;
    }));
    // disconnect calls close()
    EXPECT_CALL(*mockChannel_, close()).WillRepeatedly(Return());

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
}

// 10. RequestStateMachine — valid life cycle transitions
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

// 11. RequestStateMachine — invalid transition silently rejected
TEST_F(ModbusClientBoundaryTest, RequestState_InvalidTransition_IsRejected) {
    RequestStateMachine rsm;
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Idle);

    // Cannot go directly from Idle -> Completed
    rsm.tryTransition(RequestStateMachine::State::Completed, "invalid");
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Idle);
}

// 11b. RequestStateMachine — Failed -> Aborted transition is valid
TEST_F(ModbusClientBoundaryTest, RequestState_FailedToAborted_IsValid) {
    RequestStateMachine rsm;
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Idle);

    rsm.tryTransition(RequestStateMachine::State::Sending, "start");
    rsm.tryTransition(RequestStateMachine::State::Failed, "error");
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Failed);

    rsm.tryTransition(RequestStateMachine::State::Aborted, "abort-during-backoff");
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Aborted);

    rsm.tryTransition(RequestStateMachine::State::Idle, "reset");
    EXPECT_EQ(rsm.currentState(), RequestStateMachine::State::Idle);
}

// 12. Write failure -> error without timeout
TEST_F(ModbusClientBoundaryTest, SendRequest_WriteFails_ReturnsError) {
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(false));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
}

// 13. Oversized PDU -> validation error
TEST_F(ModbusClientBoundaryTest, SendRequest_OversizedPdu_ReturnsError) {
    connectClient();

    QByteArray hugeData(config::Modbus::kMaxAduSize + 100, 0x00);
    Pdu hugePdu(FunctionCode::ReadHoldingRegisters, hugeData);

    auto resp = client_->sendRequest(hugePdu);
    EXPECT_TRUE(resp.isError());
}

// 14. ConnectionStateMachine — valid transitions
TEST_F(ModbusClientBoundaryTest, ConnectionState_ValidTransition_Accepted) {
    ConnectionStateMachine csm;

    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Disconnected);

    csm.tryTransition(ConnectionStateMachine::State::Connecting, "connect-called");
    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Connecting);

    csm.tryTransition(ConnectionStateMachine::State::Connected, "open-ok");
    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Connected);
}

// 15. ConnectionStateMachine — invalid transition silently rejected
TEST_F(ModbusClientBoundaryTest, ConnectionState_InvalidTransition_Rejected) {
    ConnectionStateMachine csm;

    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Disconnected);

    // Cannot go from Disconnected -> Connected directly (must go through Connecting)
    csm.tryTransition(ConnectionStateMachine::State::Connected, "invalid");
    EXPECT_EQ(csm.currentState(), ConnectionStateMachine::State::Disconnected);
}

// 16. lastChannelError after connection failure
TEST_F(ModbusClientBoundaryTest, LastChannelError_AfterConnectionFailure_HasError) {
    // ensureConnected tries open() once
    Expectation openCall = EXPECT_CALL(*mockChannel_, open()).WillOnce(Return(false));
    EXPECT_CALL(*mockChannel_, close()).After(openCall);

    client_->connect();

    QString err = client_->lastChannelError();
    EXPECT_FALSE(err.isEmpty());
}

// 17. isConnected toggle
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

// 18. sendRaw on disconnected channel — no crash
TEST_F(ModbusClientBoundaryTest, SendRaw_NotConnected_DoesNotCrash) {
    QByteArray raw = QByteArray::fromHex("010300000001840A");
    EXPECT_NO_FATAL_FAILURE(client_->sendRaw(raw));
}

// 19. Abort from idle state — triggers Aborted transition
TEST_F(ModbusClientBoundaryTest, Abort_WhenIdle_DoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(client_->abort());
    // abort() transitions Idle->Aborted since Idle != Completed and != Failed
}

// Half-open detection: consecutive timeout eviction
// A session that times out unresponsiveThreshold times in a row while the
// FSM reports Connected is declared half-open and evicted: Connected ->
// Failed + channel close. The mock channel never answers, so every request
// runs the full response-wait timeout.
TEST_F(ModbusClientBoundaryTest, UnresponsiveThreshold_ClosesAndFails) {
    cfg_.mode = ModbusMode::TCP; // no RTU write-drain / frame-boundary waits
    cfg_.timeoutMs = 100;
    cfg_.unresponsiveThreshold = 3;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .Times(3)
        .WillRepeatedly(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).Times(3).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannel_, close()).Times(1); // the eviction teardown

    for (int i = 0; i < 3; ++i) {
        auto resp = client_->sendRequest(makeRhrPdu());
        EXPECT_TRUE(resp.isError());
    }

    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed)
        << "half-open session must be evicted after the threshold";
}

// A successful response resets the consecutive-timeout counter: two
// timeouts, one success, two more timeouts never reach a threshold of 3 and
// the session stays Connected with no eviction close.
TEST_F(ModbusClientBoundaryTest, UnresponsiveThreshold_SuccessResetsCounter) {
    cfg_.mode = ModbusMode::TCP;
    cfg_.timeoutMs = 100;
    cfg_.unresponsiveThreshold = 3;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .Times(5)
        .WillRepeatedly(Return(QByteArray::fromHex("010300000001840A")));
    int writeCount = 0;
    EXPECT_CALL(*mockChannel_, write(_)).Times(5).WillRepeatedly(Invoke([this, &writeCount](QByteArrayView) {
        ++writeCount;
        if (writeCount == 3) {
            feedFrameAfter(0, QByteArray::fromHex("000100000007010304000A000B"));
        }
        return true;
    }));
    EXPECT_CALL(*mockChannel_, close()).Times(0); // never evicted
    // Only the third request is answered (valid TCP MBAP frame for the
    // FC=03 / qty=2 request; the mock transport returns the matching Pdu).
    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .WillOnce(Return(ParseResponseResult{
            ParseResponseStatus::Ok,
            Pdu(FunctionCode::ReadHoldingRegisters,
                QByteArray::fromHex("04000A000B"))}));

    for (int i = 0; i < 2; ++i) { // timeouts 1..2
        EXPECT_TRUE(client_->sendRequest(makeRhrPdu()).isError());
    }

    {
        auto resp = client_->sendRequest(makeRhrPdu());
        ASSERT_FALSE(resp.isError()) << resp.error.toStdString();
    }

    for (int i = 0; i < 2; ++i) { // timeouts would be 3..4 — still below a
        EXPECT_TRUE(client_->sendRequest(makeRhrPdu()).isError()); // fresh counter
    }

    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
}

// unresponsiveThreshold = 0 disables the eviction: a timeout storm leaves
// the session Connected (legacy behavior preserved for opt-out).
TEST_F(ModbusClientBoundaryTest, UnresponsiveThreshold_DisabledWhenZero) {
    cfg_.mode = ModbusMode::TCP;
    cfg_.timeoutMs = 100;
    cfg_.unresponsiveThreshold = 0;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .Times(4)
        .WillRepeatedly(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).Times(4).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockChannel_, close()).Times(0);

    for (int i = 0; i < 4; ++i) {
        EXPECT_TRUE(client_->sendRequest(makeRhrPdu()).isError());
    }

    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
}

// On-demand connect: a request against an unopened session opens it
// Verifies that a request issued against an unopened session triggers on-demand
// connection rather than failing immediately with a disconnected error.
TEST_F(ModbusClientBoundaryTest, SendRequest_WhileDisconnected_LazyConnects) {
    cfg_.mode = ModbusMode::TCP;
    cfg_.timeoutMs = 100;
    client_->setConfig(cfg_);
    ASSERT_FALSE(client_->isConnected()); // no session yet

    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([this]() {
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(true));

    auto resp = client_->sendRequest(makeRhrPdu());
    // The request reached the response wait (timeout), proving on-demand
    // connect succeeded — the old behavior failed here immediately with
    // "Not connected" without ever dispatching the request.
    EXPECT_THAT(resp.error.toStdString(), AnyOf(HasSubstr("Timeout"), HasSubstr("timeout")));
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
    EXPECT_TRUE(client_->isConnected());
}

// Differentiated Retries: Poll reads fail fast; Control writes retry
TEST_F(ModbusClientBoundaryTest, DifferentiatedRetries_ReadPollFailsFastWithZeroRetries) {
    cfg_.retries = 0;
    cfg_.pollRetries = 0;
    cfg_.timeoutMs = 100;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(true));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
    EXPECT_EQ(resp.attemptCount, 1);
}

TEST_F(ModbusClientBoundaryTest, DifferentiatedRetries_WriteRetriesWithDefaultPolicy) {
    cfg_.retries = 0;
    cfg_.writeRetries = 1;
    cfg_.timeoutMs = 100;
    cfg_.retryIntervalMs = 5;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .Times(2)
        .WillRepeatedly(Return(QByteArray::fromHex("0110000000020400010002")));
    EXPECT_CALL(*mockChannel_, write(_)).Times(2).WillRepeatedly(Return(true));

    auto resp = client_->sendRequest(makeWmrPdu(), 1);
    EXPECT_TRUE(resp.isError());
    EXPECT_EQ(resp.attemptCount, 2);
}

// Result Interoperability: sendRequestResult and sendRequestAsyncResult
TEST_F(ModbusClientBoundaryTest, SendRequestResult_DeliversResultDirectly) {
    cfg_.mode = ModbusMode::TCP;
    cfg_.timeoutMs = 500;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("000100000006010300000002")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        feedFrameAfter(0, QByteArray::fromHex("000100000007010304000A000B"));
        return true;
    }));
    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .WillOnce(Return(ParseResponseResult{
            ParseResponseStatus::Ok,
            Pdu(FunctionCode::ReadHoldingRegisters,
                QByteArray::fromHex("04000A000B"))}));

    const auto res = client_->sendRequestResult(makeRhrPdu(), 1);

    EXPECT_TRUE(res.isOk());
    EXPECT_EQ(res.value().functionCode(), FunctionCode::ReadHoldingRegisters);
}

// Deterministic protocol exceptions (0x01..0x03) fail immediately: the
// identical request can never succeed, so no retry may be issued.
TEST_F(ModbusClientBoundaryTest, Exception02_FailsImmediatelyWithoutRetry) {
    cfg_.retries = 2;
    cfg_.retryIntervalMs = 20;
    cfg_.retryBackoffFactor = 1.0;
    cfg_.maxRetryIntervalMs = 20;
    cfg_.retryJitterPercent = 0;
    client_->setConfig(cfg_);
    connectClient();

    // Exactly one wire attempt: a retry would trip the Times(1) expectations.
    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .Times(1)
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).Times(1).WillOnce(Invoke([this](QByteArrayView) {
        writeDrainedHandler_();
        return true;
    }));
    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .Times(1)
        .WillOnce(Return(ParseResponseResult{
            ParseResponseStatus::Ok,
            Pdu(FunctionCode::ReadHoldingRegisters, ExceptionCode::IllegalDataAddress)}));

    feedFrameAfter(20, rtuFrame(0x83, QByteArray("\x02", 1)));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
    EXPECT_EQ(resp.exceptionCode, ExceptionCode::IllegalDataAddress);
    EXPECT_EQ(resp.attemptCount, 1);
}

// Device-state exceptions (0x04..0x06) keep the configured retry flow.
TEST_F(ModbusClientBoundaryTest, Exception04_StillRetriesPerStrategy) {
    cfg_.retries = 1;
    cfg_.retryIntervalMs = 20;
    cfg_.retryBackoffFactor = 1.0;
    cfg_.maxRetryIntervalMs = 20;
    cfg_.retryJitterPercent = 0;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .Times(2)
        .WillRepeatedly(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .Times(2)
        .WillRepeatedly(Return(ParseResponseResult{
            ParseResponseStatus::Ok,
            Pdu(FunctionCode::ReadHoldingRegisters, ExceptionCode::ServerDeviceFailure)}));

    // One response frame per wire attempt, each delivered relative to its own
    // request write. An absolute wall-clock schedule can fire inside the retry
    // gap while no transaction is in flight; the stray frame is then discarded
    // and the outstanding attempt times out on loaded CI runners.
    const QByteArray deviceBusyFrame = rtuFrame(0x83, QByteArray("\x04", 1));
    EXPECT_CALL(*mockChannel_, write(_))
        .Times(2)
        .WillOnce(Invoke([this, deviceBusyFrame](QByteArrayView) {
            writeDrainedHandler_();
            feedFrameAfter(20, deviceBusyFrame);
            return true;
        }))
        .WillOnce(Invoke([this, deviceBusyFrame](QByteArrayView) {
            writeDrainedHandler_();
            feedFrameAfter(20, deviceBusyFrame);
            return true;
        }));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
    EXPECT_EQ(resp.exceptionCode, ExceptionCode::ServerDeviceFailure);
    EXPECT_EQ(resp.attemptCount, 2);
}

// Exception responses go through structural validation: a malformed
// exception PDU (payload not exactly one byte) is rejected, not trusted.
TEST_F(ModbusClientBoundaryTest, MalformedExceptionPdu_RejectedAsInvalidPdu) {
    // No retries: the classification of the invalid PDU is the assertion, and
    // a retry would only observe a timeout error code instead.
    cfg_.retries = 0;
    cfg_.retryIntervalMs = 20;
    client_->setConfig(cfg_);
    connectClient();

    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .Times(1)
        .WillOnce(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockChannel_, write(_)).Times(1).WillOnce(Invoke([this](QByteArrayView) {
        writeDrainedHandler_();
        return true;
    }));
    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .Times(1)
        .WillOnce(Return(ParseResponseResult{
            ParseResponseStatus::Ok,
            Pdu(static_cast<FunctionCode>(0x83), QByteArray("\x02\x02", 2))}));

    feedFrameAfter(20, rtuFrame(0x83, QByteArray("\x02\x02", 2)));

    auto resp = client_->sendRequest(makeRhrPdu());
    EXPECT_TRUE(resp.isError());
    EXPECT_EQ(resp.errorCode, ModbusError::InvalidPdu);
}

// Transaction timeout must scale to the expected response wire time: at low
// baud a full-length response frame takes longer than the configured timeout,
// and the old fixed timeout declared the transaction dead mid-transfer.
TEST_F(ModbusClientBoundaryTest, RtuLongResponseAtLowBaud_SurvivesWireTimeScaling) {
    cfg_.retries = 0;
    cfg_.timeoutMs = 500;
    cfg_.baudRate = 300;
    cfg_.interFrameDelayUs = 0;
    cfg_.retryIntervalMs = 20;
    cfg_.retryBackoffFactor = 1.0;
    cfg_.maxRetryIntervalMs = 20;
    cfg_.retryJitterPercent = 0;
    client_->setConfig(cfg_);
    connectClient();

    // FC03 qty=16 -> 37-byte response ADU; at 300 baud (10 bits/char) the
    // frame alone takes ~1.23s on the wire, plus t3.5 silence (~117ms).
    // Scaled timeout ~1.35s; the old fixed timeout was ~617ms.
    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .WillOnce(Return(QByteArray::fromHex("01030000001005C9")));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        writeDrainedHandler_();
        return true;
    }));
    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .WillOnce(Return(ParseResponseResult{
            ParseResponseStatus::Ok,
            Pdu(FunctionCode::ReadHoldingRegisters,
                QByteArray::fromHex("20") + QByteArray(32, '\x00'))}));

    QByteArray response;
    response.append(static_cast<char>(0x01)); // slave
    response.append(static_cast<char>(0x03)); // fc
    response.append(static_cast<char>(0x20)); // byte count = 32
    response.append(QByteArray(32, '\x00'));
    const uint16_t crc = calculateModbusRtuCrc(QByteArrayView(response));
    response.append(static_cast<char>(crc & 0xFF));
    response.append(static_cast<char>((crc >> 8) & 0xFF));

    // Drip the response one byte every 20ms on the main thread's event loop:
    // gaps stay below t1.5 at 300 baud, so the extractor keeps a single
    // frame, and the transfer finishes at ~740ms — well past the old fixed
    // timeout but inside the scaled one. The synchronous request wait pumps
    // this loop; feeding from a sleeping worker thread instead would stack
    // scheduler latency on every byte and blow the inter-byte gap budget on
    // loaded CI runners.
    QTimer drip;
    int fed = 0;
    QObject::connect(&drip, &QTimer::timeout, &drip, [&]() {
        if (fed < response.size() && readHandler_) {
            readHandler_(response.mid(fed, 1));
            ++fed;
        }
        if (fed >= response.size()) {
            drip.stop();
        }
    });
    drip.start();

    auto resp = client_->sendRequest(modbus::test::MakeReadHoldingRegistersRequest(0, 16));
    drip.stop();
    ASSERT_FALSE(resp.isError()) << resp.error.toStdString();
}

// A stale send-window timer must not put bytes on the wire after abort.
TEST_F(ModbusClientBoundaryTest, AbortDuringRtuSendWindow_StaleTimerDoesNotWrite) {
    cfg_.retries = 0;
    cfg_.timeoutMs = 500;
    cfg_.baudRate = 300; // t3.5 silence ~117ms arms the deferred send window
    cfg_.interFrameDelayUs = 0;
    client_->setConfig(cfg_);
    connectClient();

    // Request #1 completes immediately and opens the RTU send window.
    EXPECT_CALL(*mockTransport_, buildRequest(_, _))
        .Times(2)
        .WillRepeatedly(Return(QByteArray::fromHex("010300000001840A")));
    EXPECT_CALL(*mockTransport_, parseResponse(_))
        .WillOnce(Return(ParseResponseResult{
            ParseResponseStatus::Ok,
            Pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("04000A000B"))}));
    // Exactly one wire write: request #2 is aborted while waiting for the
    // send window, so its deferred write must never happen.
    EXPECT_CALL(*mockChannel_, write(_)).Times(1).WillOnce(Invoke([this](QByteArrayView) {
        writeDrainedHandler_();
        return true;
    }));

    feedFrameAfter(20, rtuFrame(0x03, QByteArray::fromHex("04000A000B")));

    auto first = client_->sendRequest(makeRhrPdu());
    ASSERT_FALSE(first.isError()) << first.error.toStdString();

    bool callbackFired = false;
    std::optional<ModbusResponse> secondResponse;
    client_->sendRequestAsync(makeRhrPdu(), 1, 0,
                              [&](const ModbusResponse& response) {
                                  callbackFired = true;
                                  secondResponse = response;
                              });
    client_->abort();

    EXPECT_TRUE(callbackFired);
    ASSERT_TRUE(secondResponse.has_value());
    EXPECT_TRUE(secondResponse->isError());

    // Pump the loop past the would-be send-window deadline: any stale timer
    // fire would trip the Times(1) write expectation above.
    QTest::qWait(200);
}

} // namespace
