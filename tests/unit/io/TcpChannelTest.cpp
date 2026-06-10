#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../../core/io/TcpChannel.h"
#include "../../helpers/ModbusTestHelpers.h"

using namespace testing;
using namespace io;

namespace {

class TcpChannelTest : public Test {
protected:
    void SetUp() override {
        channel_ = std::make_unique<TcpChannel>();
    }

    void TearDown() override {
        // Close first to clean up socket state before destruction
        if (channel_) {
            channel_->close();
        }
        channel_.reset();
    }

    std::unique_ptr<TcpChannel> channel_;
};

// ---------------------------------------------------------------------------
// Default construction state
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, Construct_DefaultKindIsTcp) {
    EXPECT_EQ(channel_->kind(), ChannelKind::Tcp);
}

TEST_F(TcpChannelTest, Construct_DefaultStateIsClosed) {
    EXPECT_EQ(channel_->state(), ChannelState::Closed);
}

TEST_F(TcpChannelTest, Construct_DefaultNotOpen) {
    EXPECT_FALSE(channel_->isOpen());
}

// ---------------------------------------------------------------------------
// Endpoint configuration
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, SetEndpoint_ValidIpPort_NoCrash) {
    EXPECT_NO_FATAL_FAILURE(channel_->setEndpoint(QStringLiteral("192.168.1.100"), 502));
}

TEST_F(TcpChannelTest, SetEndpoint_Localhost_NoCrash) {
    EXPECT_NO_FATAL_FAILURE(channel_->setEndpoint(QStringLiteral("127.0.0.1"), 1502));
}

// ---------------------------------------------------------------------------
// Write on closed channel
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, Write_NoConnection_ReturnsFalse) {
    QByteArray testData = QByteArrayLiteral("test");
    bool wrote = channel_->write(testData);
    EXPECT_FALSE(wrote);
}

// ---------------------------------------------------------------------------
// Close from closed state
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, Close_FromClosedState_NoCrash) {
    EXPECT_NO_FATAL_FAILURE(channel_->close());
    EXPECT_EQ(channel_->state(), ChannelState::Closed);
}

// ---------------------------------------------------------------------------
// Timeouts
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, DefaultTimeouts_HaveExpectedValues) {
    Timeouts t = channel_->timeouts();
    EXPECT_GT(t.readMs, 0);
    EXPECT_GT(t.writeMs, 0);
}

TEST_F(TcpChannelTest, SetTimeouts_CustomValues_Persist) {
    Timeouts custom;
    custom.readMs = 500;
    custom.writeMs = 1000;
    channel_->setTimeouts(custom);

    Timeouts retrieved = channel_->timeouts();
    EXPECT_EQ(retrieved.readMs, 500);
    EXPECT_EQ(retrieved.writeMs, 1000);
}

// ---------------------------------------------------------------------------
// Stats
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, Stats_InitialZero) {
    ChannelStats s = channel_->stats();
    EXPECT_EQ(s.bytesTx, 0);
    EXPECT_EQ(s.bytesRx, 0);
}

// ---------------------------------------------------------------------------
// Handlers — verify they can be set without crash
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, SetReadHandler_NoCrash) {
    bool called = false;
    channel_->setReadHandler([&called](QByteArrayView) { called = true; });
    SUCCEED();
}

TEST_F(TcpChannelTest, SetErrorHandler_NoCrash) {
    bool called = false;
    channel_->setErrorHandler([&called](const QString&) { called = true; });
    SUCCEED();
}

TEST_F(TcpChannelTest, AddStateHandler_ReturnsNonZero) {
    auto id = channel_->addStateHandler([](ChannelState) {});
    EXPECT_NE(id, 0);
    channel_->removeStateHandler(id);
}

TEST_F(TcpChannelTest, RemoveStateHandler_NoCrash) {
    auto id = channel_->addStateHandler([](ChannelState) {});
    EXPECT_NO_FATAL_FAILURE(channel_->removeStateHandler(id));
}

} // namespace