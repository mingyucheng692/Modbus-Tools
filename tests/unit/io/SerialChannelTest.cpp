#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../../infra/io/SerialChannel.h"
#include "../../../infra/io/SerialConfig.h"
#include "../../helpers/ModbusTestHelpers.h"

using namespace testing;
using namespace io;

namespace {

class SerialChannelTest : public Test {
protected:
    void SetUp() override {
        channel_ = std::make_unique<SerialChannel>();
    }

    void TearDown() override {
        if (channel_) {
            channel_->close();
        }
        channel_.reset();
    }

    std::unique_ptr<SerialChannel> channel_;
};

// ---------------------------------------------------------------------------
// Default construction state
// ---------------------------------------------------------------------------

TEST_F(SerialChannelTest, Construct_DefaultKindIsSerial) {
    EXPECT_EQ(channel_->kind(), ChannelKind::Serial);
}

TEST_F(SerialChannelTest, Construct_DefaultStateIsClosed) {
    EXPECT_EQ(channel_->state(), ChannelState::Closed);
}

TEST_F(SerialChannelTest, Construct_DefaultNotOpen) {
    EXPECT_FALSE(channel_->isOpen());
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

TEST_F(SerialChannelTest, SetConfig_ValidConfig_NoCrash) {
    SerialConfig cfg;
    cfg.portName = QStringLiteral("COM1");
    cfg.baudRate = 9600;
    cfg.dataBits = 8;
    cfg.stopBits = 1;
    cfg.parity = QSerialPort::NoParity;
    cfg.flowControl = QSerialPort::NoFlowControl;

    EXPECT_NO_FATAL_FAILURE(channel_->setConfig(cfg));
}

TEST_F(SerialChannelTest, SetConfig_ChangeBaudRate_NoCrash) {
    SerialConfig cfg;
    cfg.portName = QStringLiteral("COM2");
    cfg.baudRate = 115200;

    EXPECT_NO_FATAL_FAILURE(channel_->setConfig(cfg));
}

// ---------------------------------------------------------------------------
// DTR / RTS control (should not crash even without open port)
// ---------------------------------------------------------------------------

TEST_F(SerialChannelTest, DtrControl_NoCrash) {
    EXPECT_NO_FATAL_FAILURE(channel_->setDtr(true));
    EXPECT_NO_FATAL_FAILURE(channel_->setDtr(false));
}

TEST_F(SerialChannelTest, RtsControl_NoCrash) {
    EXPECT_NO_FATAL_FAILURE(channel_->setRts(true));
    EXPECT_NO_FATAL_FAILURE(channel_->setRts(false));
}

// ---------------------------------------------------------------------------
// Close with no open channel
// ---------------------------------------------------------------------------

TEST_F(SerialChannelTest, Close_FromClosedState_NoCrash) {
    EXPECT_NO_FATAL_FAILURE(channel_->close());
    EXPECT_EQ(channel_->state(), ChannelState::Closed);
}

// ---------------------------------------------------------------------------
// Timeouts
// ---------------------------------------------------------------------------

TEST_F(SerialChannelTest, DefaultTimeouts_HaveExpectedValues) {
    Timeouts t = channel_->timeouts();
    EXPECT_GT(t.readMs, 0);
    EXPECT_GT(t.writeMs, 0);
}

TEST_F(SerialChannelTest, SetTimeouts_CustomValues_Persist) {
    Timeouts custom;
    custom.readMs = 200;
    custom.writeMs = 300;
    channel_->setTimeouts(custom);

    Timeouts retrieved = channel_->timeouts();
    EXPECT_EQ(retrieved.readMs, 200);
    EXPECT_EQ(retrieved.writeMs, 300);
}

// ---------------------------------------------------------------------------
// Stats
// ---------------------------------------------------------------------------

TEST_F(SerialChannelTest, Stats_InitialZero) {
    ChannelStats s = channel_->stats();
    EXPECT_EQ(s.bytesTx, 0);
    EXPECT_EQ(s.bytesRx, 0);
}

} // namespace