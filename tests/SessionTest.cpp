#include <gtest/gtest.h>
#include "modbus/session/ModbusClient.h"
#include "io/IChannel.h"
#include "modbus/transport/ITransport.h"
#include <QCoreApplication>
#include <QTimer>
#include <thread>

using namespace modbus::session;
using namespace modbus::base;
using namespace modbus::transport;
using namespace io;

// Simple Mock Channel
class MockChannel : public IChannel {
public:
    ChannelKind kind() const override { return ChannelKind::Tcp; }
    ChannelState state() const override { return state_; }
    bool open() override { state_ = ChannelState::Open; if(stateHandler_) stateHandler_(state_); return true; }
    void close() override { state_ = ChannelState::Closed; if(stateHandler_) stateHandler_(state_); }
    bool isOpen() const override { return state_ == ChannelState::Open; }
    void setTimeouts(const Timeouts& t) override { timeouts_ = t; }
    Timeouts timeouts() const override { return timeouts_; }
    void moveToThread(QThread*) override {}
    
    bool write(QByteArrayView data) override {
        lastWritten_ = QByteArray(data.data(), data.size());
        if (autoResponse_) {
            // Trigger write drained immediately to satisfy ModbusClient's wait
            if (writeDrainedHandler_) writeDrainedHandler_();
            
            // Simulate async response in a separate thread
            std::thread([this](){
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if (readHandler_) readHandler_(autoResponseData_);
            }).detach();
        }
        return true;
    }

    void setReadHandler(std::function<void(QByteArrayView)> h) override { readHandler_ = h; }
    void setErrorHandler(std::function<void(const QString&)> h) override { errorHandler_ = h; }
    void setWriteDrainedHandler(std::function<void()> h) override { writeDrainedHandler_ = h; }
    void setStateHandler(std::function<void(ChannelState)> h) override { stateHandler_ = h; }
    HandlerId addStateHandler(std::function<void(ChannelState)> h) override { stateHandler_ = h; return 1; }
    void removeStateHandler(HandlerId) override {}
    void setMonitor(std::function<void(bool, const QByteArray&)>) override {}
    ChannelStats stats() const override { return {}; }

    // Control methods for tests
    void simulateResponse(const QByteArray& data) { if(readHandler_) readHandler_(data); }
    void setAutoResponse(bool enable, const QByteArray& data = {}) { autoResponse_ = enable; autoResponseData_ = data; }

    ChannelState state_ = ChannelState::Closed;
    Timeouts timeouts_;
    QByteArray lastWritten_;
    std::function<void(QByteArrayView)> readHandler_;
    std::function<void(const QString&)> errorHandler_;
    std::function<void()> writeDrainedHandler_;
    std::function<void(ChannelState)> stateHandler_;
    bool autoResponse_ = false;
    QByteArray autoResponseData_;
};

// Simple Mock Transport
class MockTransport : public ITransport {
public:
    QByteArray buildRequest(const Pdu& pdu, uint8_t slaveId) override {
        lastPdu_ = pdu;
        return QByteArray(1, slaveId) + pdu.toByteArray();
    }
    ParseResponseResult parseResponse(const QByteArray& adu) override {
        if (adu.size() < 2) return {ParseResponseStatus::Invalid, std::nullopt};
        FunctionCode fc = static_cast<FunctionCode>(adu[1]);
        QByteArray data = adu.mid(2);
        return {ParseResponseStatus::Ok, Pdu(fc, data)};
    }
    int checkIntegrity(const QByteArray& data) override {
        return data.size() >= 2 ? data.size() : 0;
    }
    void resetPendingState() override {}
    
    Pdu lastPdu_;
};

class SessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        mockChannel_ = std::make_shared<MockChannel>();
        mockTransport_ = std::make_shared<MockTransport>();
        client_ = std::make_unique<ModbusClient>(mockChannel_, mockTransport_);
        
        ModbusConfig config;
        config.mode = ModbusMode::TCP;
        config.timeoutMs = 100; 
        config.retries = 1;
        client_->setConfig(config);
    }

    std::shared_ptr<MockChannel> mockChannel_;
    std::shared_ptr<MockTransport> mockTransport_;
    std::unique_ptr<ModbusClient> client_;
};

TEST_F(SessionTest, SuccessfulRequest) {
    client_->connect();
    
    // Simulate valid response: Slave 1, FC 3, Data 00 7B (123)
    mockChannel_->setAutoResponse(true, QByteArray::fromHex("010302007B"));
    
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    ModbusResponse response = client_->sendRequest(request, 1);
    
    EXPECT_TRUE(response.isSuccess);
    EXPECT_EQ(response.pdu.functionCode(), FunctionCode::ReadHoldingRegisters);
}

TEST_F(SessionTest, TimeoutHandling) {
    client_->connect();
    mockChannel_->setAutoResponse(false); // No response
    
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    ModbusResponse response = client_->sendRequest(request, 1);
    
    EXPECT_FALSE(response.isSuccess);
    EXPECT_TRUE(response.error.contains("timeout", Qt::CaseInsensitive));
}

TEST_F(SessionTest, RetryLogic) {
    client_->connect();
    
    // Configure fast retry backoff
    ModbusConfig config;
    config.mode = ModbusMode::TCP;
    config.timeoutMs = 100;
    config.retries = 1;
    config.retryIntervalMs = 50; 
    client_->setConfig(config);
    
    std::thread responder([this](){
        // First attempt happens at 0-100ms
        // Backoff happens at 100-150ms
        // Second attempt starts at 150ms
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
        mockChannel_->simulateResponse(QByteArray::fromHex("010302007B"));
    });
    
    Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    ModbusResponse response = client_->sendRequest(request, 1);
    
    EXPECT_TRUE(response.isSuccess);
    if (responder.joinable()) responder.join();
}

TEST_F(SessionTest, RequestQueueExecution) {
    client_->connect();
    mockChannel_->setAutoResponse(true, QByteArray::fromHex("010302007B"));
    
    // Send multiple requests and ensure they don't crash and maintain order
    for (int i = 0; i < 5; ++i) {
        Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
        ModbusResponse response = client_->sendRequest(request, 1);
        EXPECT_TRUE(response.isSuccess);
    }
}
