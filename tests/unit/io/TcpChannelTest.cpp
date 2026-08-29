#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTcpServer>
#include <QtTest>

#include "../../../infra/io/TcpChannel.h"
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

// ---------------------------------------------------------------------------
// TCP endpoint validation
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, TcpChannel_InvalidIpAddress_ReturnsError) {
    bool errorEmitted = false;
    channel_->setErrorHandler([&errorEmitted](const QString&) {
        errorEmitted = true;
    });

    channel_->setEndpoint(QStringLiteral("not.an.ip"), 502);
    channel_->open();

    EXPECT_TRUE(errorEmitted);
}

TEST_F(TcpChannelTest, TcpChannel_InvalidPort_ReturnsError) {
    bool errorEmitted = false;
    channel_->setErrorHandler([&errorEmitted](const QString&) {
        errorEmitted = true;
    });

    channel_->setEndpoint(QStringLiteral("127.0.0.1"), 0);
    channel_->open();

    EXPECT_TRUE(errorEmitted);
}

TEST_F(TcpChannelTest, TcpChannel_ValidParams_OpensSuccessfully) {
    bool errorEmitted = false;
    channel_->setErrorHandler([&errorEmitted](const QString&) {
        errorEmitted = true;
    });

    channel_->setEndpoint(QStringLiteral("127.0.0.1"), 1502);
    channel_->open();

    EXPECT_FALSE(errorEmitted);
}

// ---------------------------------------------------------------------------
// T3.1 — Error is sticky: once the channel FSM is in Error, late socket
// state notifications must not repaint it into a healthy Closed/Open.
// Error is left only via open() (new attempt) or close() (user teardown).
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, Error_LeavesOnlyViaOpenOrClose) {
    // Land the FSM in Error through the synchronous invalid-endpoint path.
    channel_->setEndpoint(QStringLiteral("not.an.ip"), 502);
    channel_->open();
    ASSERT_EQ(channel_->state(), ChannelState::Error);

    // close() is a legal exit from Error (user-initiated teardown).
    channel_->close();
    EXPECT_EQ(channel_->state(), ChannelState::Closed);

    // open() is a legal exit from Closed (fresh attempt).
    channel_->setEndpoint(QStringLiteral("127.0.0.1"), 1502);
    channel_->open();
    EXPECT_NE(channel_->state(), ChannelState::Error);
}

TEST_F(TcpChannelTest, SocketError_StickyAgainstLateSocketStateChange) {
    // T3.1: after a socket error lands the FSM in Error, the late queued
    // socket stateChanged(UnconnectedState) that follows the aborted
    // connection must NOT repaint Error into a healthy Closed.
    // (Before the sticky guard the repaint silently converted failures
    // into "clean" disconnects.)
    bool errorEmitted = false;
    channel_->setErrorHandler([&errorEmitted](const QString&) {
        errorEmitted = true;
    });

    // Reserve then release an ephemeral loopback port: nothing listens on
    // it afterwards, so the OS answers the connect attempt with an
    // immediate RST (ConnectionRefused) — deterministic on every machine,
    // no external network dependency.
    QTcpServer probe;
    ASSERT_TRUE(probe.listen(QHostAddress::LocalHost, 0));
    const quint16 refusedPort = probe.serverPort();
    probe.close();

    io::Timeouts t;
    t.readMs = 1000;
    t.writeMs = 1000;
    channel_->setTimeouts(t);
    channel_->setEndpoint(QStringLiteral("127.0.0.1"),
                          static_cast<int>(refusedPort));

    channel_->open();
    // QTest::qWait pumps the event loop while sleeping, so the queued
    // ConnectionRefused notification is actually delivered (a bare
    // processEvents() spin only handles events that are already posted
    // and can exit before the OS reports the failure).
    for (int i = 0; i < 100 && !errorEmitted; ++i) {
        QTest::qWait(20);
    }
    ASSERT_TRUE(errorEmitted);
    ASSERT_EQ(channel_->state(), ChannelState::Error);

    // Drain every queued socket notification following the failure: Error
    // must survive all of them.
    QTest::qWait(100);
    for (int i = 0; i < 10; ++i) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    EXPECT_EQ(channel_->state(), ChannelState::Error)
        << "late socket state change repainted sticky Error state";
}

// ---------------------------------------------------------------------------
// T3.2 — close linger fallback: a close that cannot complete (kernel send
// buffer still holds unflushed data to a non-reading peer) must be
// force-completed after the linger timeout instead of staying in Closing.
// ---------------------------------------------------------------------------

TEST_F(TcpChannelTest, Close_StuckFlush_ForcedClosedAfterLinger) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));
    // NOTE: accept the connection but never read from it.

    TcpChannel channel(100); // short linger for test velocity
    channel.setEndpoint(QStringLiteral("127.0.0.1"),
                       static_cast<int>(server.serverPort()));
    channel.open();

    // Wait for the connection to be established (queued socket signals;
    // qWait sleeps while pumping the loop so notifications are delivered).
    for (int i = 0; i < 100 && channel.state() != ChannelState::Open; ++i) {
        QTest::qWait(20);
    }
    ASSERT_EQ(channel.state(), ChannelState::Open);

    // Overwhelm the kernel send buffer + peer receive window so the
    // graceful disconnectFromHost() cannot complete synchronously.
    const QByteArray block(64 * 1024, 'x');
    for (int i = 0; i < 128; ++i) { // ~8 MiB, far beyond any SO_SNDBUF
        channel.write(block);
    }

    channel.close();

    // Either the graceful close completes, or the linger timer
    // force-aborts at ~100ms — either way Closed must be reached.
    for (int i = 0; i < 100 && channel.state() != ChannelState::Closed; ++i) {
        QTest::qWait(20);
    }
    EXPECT_EQ(channel.state(), ChannelState::Closed)
        << "close() stranded the channel in Closing (linger fallback failed)";
}

} // namespace