#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "modbus/session/ModbusClient.h"
#include "../../mocks/MockChannel.h"
#include "../../mocks/MockTransport.h"
#include <thread>
#include <future>
#include <chrono>

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
        
        client_ = std::make_unique<ModbusClient>(mockChannel_, mockTransport_);
        
        modbus::base::ModbusConfig config;
        config.mode = ModbusMode::TCP;
        config.timeoutMs = 500; 
        config.retries = 1;
        client_->setConfig(config);

        ON_CALL(*mockChannel_, state()).WillByDefault(ReturnPointee(&currentChannelState_));
        ON_CALL(*mockChannel_, isOpen()).WillByDefault(Invoke([this](){ return currentChannelState_ == ChannelState::Open; }));
        
        ON_CALL(*mockTransport_, buildRequest(_, _)).WillByDefault(Return(QByteArray::fromHex("010300000001")));
        ON_CALL(*mockTransport_, parseResponse(_)).WillByDefault(Return(ParseResponseResult{ParseResponseStatus::Ok, Pdu(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("007B"))}));

        // Trigger successful connection setup by default
        EXPECT_CALL(*mockChannel_, addStateHandler(_)).WillRepeatedly(DoAll(SaveArg<0>(&stateHandler_), Return(1)));
        EXPECT_CALL(*mockChannel_, open()).WillRepeatedly(Invoke([this](){
            currentChannelState_ = ChannelState::Open;
            if (stateHandler_) stateHandler_(ChannelState::Open);
            return true;
        }));
        
        // Suppress mock leaks for integration tests as we handle destruction manually
        Mock::AllowLeak(mockChannel_.get());
        Mock::AllowLeak(mockTransport_.get());
    }

    void TearDown() override {
        // Joining all simulation threads before destroying objects
        simulationThreads_.clear();
        
        if (client_) {
            client_->abort(); // Secure shutdown for industrial standard
            client_.reset();
        }
        mockChannel_.reset();
        mockTransport_.reset();
    }

    ChannelState currentChannelState_ = ChannelState::Closed;
    std::shared_ptr<NiceMock<MockChannel>> mockChannel_;
    std::shared_ptr<NiceMock<MockTransport>> mockTransport_;
    std::unique_ptr<ModbusClient> client_;
    std::vector<std::jthread> simulationThreads_;
    std::function<void(ChannelState)> stateHandler_;
    std::function<void(QByteArrayView)> readHandler_;
};

TEST_F(ClientIntegrationTest, SuccessfulRequestAsync) {
    client_->connect(); 
    
    EXPECT_CALL(*mockChannel_, setReadHandler(_)).WillRepeatedly(SaveArg<0>(&readHandler_));
    
    // Re-trigger connect logic to ensure handlers are captured
    client_->connect(); 

    // ASYNC SIMULATION: 
    // Trigger the mock response with a small delay to simulate real hardware latency.
    // This allows the client state machine to reach its wait loop properly.
    EXPECT_CALL(*mockChannel_, write(_)).WillOnce(Invoke([this](QByteArrayView){
        simulationThreads_.emplace_back([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (this->readHandler_) {
                QByteArray data = QByteArray::fromHex("010302007B");
                this->readHandler_(data);
            }
        });
        return true;
    }));
    
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    ModbusResponse response = client_->sendRequest(request, 1);
    
    EXPECT_TRUE(response.isSuccess);
    EXPECT_EQ(response.pdu.functionCode(), FunctionCode::ReadHoldingRegisters);
}

TEST_F(ClientIntegrationTest, TimeoutHandlingAsync) {
    client_->connect();
    
    // No mock response will be triggered, expecting timeout
    EXPECT_CALL(*mockChannel_, write(_)).WillRepeatedly(Return(true));
    
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    ModbusResponse response = client_->sendRequest(request, 1);
    
    EXPECT_FALSE(response.isSuccess);
}

TEST_F(ClientIntegrationTest, RetryLogicAsync) {
    client_->connect(); 
    EXPECT_CALL(*mockChannel_, setReadHandler(_)).WillRepeatedly(SaveArg<0>(&readHandler_));
    client_->connect();

    // Call 1: Timeout (no async callback)
    // Call 2: Success
    EXPECT_CALL(*mockChannel_, write(_)).Times(2)
        .WillOnce(Return(true)) 
        .WillOnce(Invoke([this](QByteArrayView){
            simulationThreads_.emplace_back([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                if (this->readHandler_) {
                    QByteArray data = QByteArray::fromHex("010302007B");
                    this->readHandler_(data);
                }
            });
            return true;
        }));
    
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    ModbusResponse response = client_->sendRequest(request, 1);
    
    EXPECT_TRUE(response.isSuccess);
}
