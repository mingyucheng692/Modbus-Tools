/**
 * @file ClientIntegrationTest.cpp
 * @brief Event-driven client integration tests with valid TCP ADUs and mocked I/O.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "modbus/session/ModbusClient.h"
#include "modbus/base/ModbusAduBuilder.h"
#include "logging/TraceContext.h"
#include "../../mocks/MockChannel.h"
#include "../../mocks/MockTransport.h"
#include <QObject>
#include <QTimer>

using namespace modbus::session;
using namespace modbus::base;
using namespace modbus::transport;
using namespace io;
using namespace testing;

class ClientIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockChannel_ = std::make_shared<NiceMock<MockChannel>>();
        mockTransport_ = std::make_shared<NiceMock<MockTransport>>();
        EXPECT_CALL(*mockChannel_, setReadHandler(_)).WillRepeatedly(SaveArg<0>(&readHandler_));
        EXPECT_CALL(*mockChannel_, addStateHandler(_)).WillRepeatedly(DoAll(SaveArg<0>(&stateHandler_), Return(1)));
        client_ = std::make_unique<ModbusClient>(mockChannel_, mockTransport_);

        ModbusConfig config;
        config.mode = ModbusMode::TCP;
        config.timeoutMs = 100;
        config.retries = 1;
        config.retryIntervalMs = 1;
        client_->setConfig(config);
        ON_CALL(*mockChannel_, state()).WillByDefault(ReturnPointee(&currentChannelState_));
        ON_CALL(*mockChannel_, isOpen()).WillByDefault(Invoke([this]() { return currentChannelState_ == ChannelState::Open; }));
        ON_CALL(*mockTransport_, buildRequest(_, _)).WillByDefault(Return(QByteArray::fromHex("000100000006010300000001")));
        ON_CALL(*mockTransport_, parseResponse(_)).WillByDefault(Return(ParseResponseResult{
            ParseResponseStatus::Ok, Pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("02007B"))}));
        ON_CALL(*mockTransport_, checkIntegrity(_)).WillByDefault(Invoke([](const QByteArray& data) { return data.size(); }));
        EXPECT_CALL(*mockChannel_, open()).WillRepeatedly(Invoke([this]() {
            currentChannelState_ = ChannelState::Open;
            if (stateHandler_) stateHandler_(ChannelState::Open);
            return true;
        }));
    }

    void TearDown() override {
        client_->abort();
        client_.reset();
        mockChannel_.reset();
        mockTransport_.reset();
    }

    void queueResponse() {
        // Deliver in the client's event loop after write returns. No sleeps,
        // cross-thread callbacks or leaked mocks; the context cancels on teardown.
        QTimer::singleShot(0, &callbackContext_, [this]() {
            if (readHandler_) {
                const QByteArray response = buildTcpAdu(1,
                    Pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("02007B")), 1);
                readHandler_(response);
            }
        });
    }

    QObject callbackContext_;
    ChannelState currentChannelState_ = ChannelState::Closed;
    std::shared_ptr<NiceMock<MockChannel>> mockChannel_;
    std::shared_ptr<NiceMock<MockTransport>> mockTransport_;
    std::unique_ptr<ModbusClient> client_;
    std::function<void(QByteArrayView)> readHandler_;
    std::function<void(ChannelState)> stateHandler_;
};

TEST_F(ClientIntegrationTest, SuccessfulRequestAsync) {
    client_->connect();
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        queueResponse();
        return true;
    }));
    modbus::trace::Scope trace(1);
    const auto response = client_->sendRequest(Pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001")), 1);
    EXPECT_FALSE(response.isError());
    EXPECT_EQ(response.pdu.functionCode(), FunctionCode::ReadHoldingRegisters);
}

TEST_F(ClientIntegrationTest, TimeoutHandlingAsync) {
    client_->connect();
    EXPECT_CALL(*mockChannel_, write(_)).Times(2).WillRepeatedly(Return(true));
    modbus::trace::Scope trace(1);
    const auto response = client_->sendRequest(Pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001")), 1);
    EXPECT_TRUE(response.isError());
}

TEST_F(ClientIntegrationTest, RetryLogicAsync) {
    client_->connect();
    InSequence sequence;
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView) {
        queueResponse();
        return true;
    }));
    modbus::trace::Scope trace(1);
    const auto response = client_->sendRequest(Pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001")), 1);
    EXPECT_FALSE(response.isError());
    EXPECT_EQ(response.attemptCount, 2);
}
