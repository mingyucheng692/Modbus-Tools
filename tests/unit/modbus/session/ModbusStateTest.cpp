#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "modbus/session/ModbusClient.h"
#include "../../../mocks/MockChannel.h"
#include "../../../mocks/MockTransport.h"
#include <future>

using namespace modbus::session;
using namespace modbus::transport;
using namespace modbus::base;
using namespace io;
using namespace testing;

class ModbusStateTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockChannel_ = std::make_shared<NiceMock<MockChannel>>();
        mockTransport_ = std::make_shared<NiceMock<MockTransport>>();

        ON_CALL(*mockChannel_, addStateHandler(_))
            .WillByDefault(DoAll(SaveArg<0>(&stateHandler_), Return(1)));
        ON_CALL(*mockChannel_, setReadHandler(_))
            .WillByDefault(SaveArg<0>(&readHandler_));
        ON_CALL(*mockChannel_, setErrorHandler(_))
            .WillByDefault(SaveArg<0>(&errorHandler_));
        ON_CALL(*mockChannel_, setWriteDrainedHandler(_))
            .WillByDefault(SaveArg<0>(&writeDrainedHandler_));

        client_ = std::make_unique<ModbusClient>(mockChannel_, mockTransport_);
        
        modbus::base::ModbusConfig config;
        config.timeoutMs = 500;
        client_->setConfig(config);

        // Default behavior for mocks to simulate state
        ON_CALL(*mockChannel_, state()).WillByDefault(ReturnPointee(&currentState_));
        ON_CALL(*mockChannel_, isOpen()).WillByDefault(Invoke([this](){
            return currentState_ == ChannelState::Open;
        }));
    }

    void TearDown() override {
        client_.reset();
        mockChannel_.reset();
        mockTransport_.reset();
    }

    ChannelState currentState_ = ChannelState::Closed;
    std::function<void(ChannelState)> stateHandler_;
    std::function<void(QByteArrayView)> readHandler_;
    std::function<void(const QString&)> errorHandler_;
    std::function<void()> writeDrainedHandler_;
    std::shared_ptr<NiceMock<MockChannel>> mockChannel_;
    std::shared_ptr<NiceMock<MockTransport>> mockTransport_;
    std::unique_ptr<ModbusClient> client_;
};

TEST_F(ModbusStateTest, InitialState) {
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Disconnected);
    EXPECT_EQ(client_->requestState(), ModbusClient::RequestState::Idle);
}

TEST_F(ModbusStateTest, ConnectionTransition) {
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&](){
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));

    bool ok = client_->connect();
    EXPECT_TRUE(ok);
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
}

TEST_F(ModbusStateTest, ConnectionFailure) {
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&](){
        currentState_ = ChannelState::Error;
        return false;
    }));
    bool ok = client_->connect();
    EXPECT_FALSE(ok);
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
}

TEST_F(ModbusStateTest, ManualDisconnect) {
    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&](){
        currentState_ = ChannelState::Open;
        if (stateHandler_) stateHandler_(ChannelState::Open);
        return true;
    }));
    
    client_->connect();
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
    
    EXPECT_CALL(*mockChannel_, close()).WillOnce(Invoke([&](){
        currentState_ = ChannelState::Closed;
        if (stateHandler_) stateHandler_(ChannelState::Closed);
    }));
    
    client_->disconnect();
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Disconnected);
}

TEST_F(ModbusStateTest, ConnectWaitsForAsyncStateHandlerWithoutProcessEvents) {
    std::promise<void> openCalled;
    auto openObserved = openCalled.get_future();

    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        openCalled.set_value();
        return true;
    }));

    auto connectFuture = std::async(std::launch::async, [this]() {
        return client_->connect();
    });

    ASSERT_EQ(openObserved.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    currentState_ = ChannelState::Open;
    ASSERT_TRUE(static_cast<bool>(stateHandler_));
    stateHandler_(ChannelState::Open);

    EXPECT_TRUE(connectFuture.get());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Connected);
}

TEST_F(ModbusStateTest, AbortUnblocksConnectWaitWithoutProcessEvents) {
    std::promise<void> openCalled;
    auto openObserved = openCalled.get_future();

    EXPECT_CALL(*mockChannel_, open()).WillOnce(Invoke([&]() {
        openCalled.set_value();
        return true;
    }));

    auto connectFuture = std::async(std::launch::async, [this]() {
        return client_->connect();
    });

    ASSERT_EQ(openObserved.wait_for(std::chrono::seconds(1)), std::future_status::ready);
    client_->abort();

    EXPECT_FALSE(connectFuture.get());
    EXPECT_EQ(client_->connectionState(), ModbusClient::ConnectionState::Failed);
}
