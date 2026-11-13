// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QMetaObject>
#include <QPointer>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>

#include <functional>
#include <memory>

#include "../../../infra/io/TcpChannel.h"
#include "common/ThreadGuard.h"

using namespace testing;
using namespace io;

namespace {

class ChannelSignalRelay : public QObject {
    Q_OBJECT
public:
    explicit ChannelSignalRelay(QObject* parent = nullptr) : QObject(parent) {
        qRegisterMetaType<io::ChannelState>("io::ChannelState");
        qRegisterMetaType<io::ChannelErrorCode>("io::ChannelErrorCode");
    }

    ~ChannelSignalRelay() override {
        detach();
    }

    void attach(TcpChannel& channel) {
        channel_ = &channel;
        stateHandlerId_ = channel.addStateHandler([this](ChannelState s) {
            QMetaObject::invokeMethod(this, [this, s]() {
                emit stateChanged(s);
            }, Qt::QueuedConnection);
        });
        channel.setErrorHandler([this](const ChannelError& err) {
            const ChannelErrorCode code = err.code;
            const QString msg = err.message;
            QMetaObject::invokeMethod(this, [this, code, msg]() {
                emit errorOccurred(code, msg);
            }, Qt::QueuedConnection);
        });
        channel.setReadHandler([this](QByteArrayView data) {
            const QByteArray copy(data.data(), data.size());
            QMetaObject::invokeMethod(this, [this, copy]() {
                emit dataReceived(copy);
            }, Qt::QueuedConnection);
        });
        channel.setWriteDrainedHandler([this]() {
            QMetaObject::invokeMethod(this, [this]() {
                emit writeDrained();
            }, Qt::QueuedConnection);
        });
    }

    void detach() {
        if (!channel_) {
            return;
        }
        channel_->removeStateHandler(stateHandlerId_);
        stateHandlerId_ = 0;
        channel_->setErrorHandler(nullptr);
        channel_->setReadHandler(nullptr);
        channel_->setWriteDrainedHandler(nullptr);
        channel_ = nullptr;
    }

signals:
    void stateChanged(io::ChannelState state);
    void errorOccurred(io::ChannelErrorCode code, const QString& message);
    void dataReceived(const QByteArray& data);
    void writeDrained();

private:
    TcpChannel* channel_ = nullptr;
    IChannel::HandlerId stateHandlerId_ = 0;
};

bool waitForSpyCondition(QSignalSpy& spy,
                         const std::function<bool()>& predicate,
                         int timeoutMs = 3000) {
    if (predicate()) {
        return true;
    }
    QElapsedTimer elapsed;
    elapsed.start();
    while (!predicate()) {
        const int remaining = timeoutMs - static_cast<int>(elapsed.elapsed());
        if (remaining <= 0) {
            return false;
        }
        spy.wait(remaining);
    }
    return true;
}

bool waitForChannelState(const TcpChannel& channel,
                         QSignalSpy& stateSpy,
                         ChannelState target,
                         int timeoutMs = 3000) {
    return waitForSpyCondition(stateSpy, [&channel, &stateSpy, target]() {
        if (channel.state() == target) {
            return true;
        }
        for (const auto& args : stateSpy) {
            if (args.at(0).value<ChannelState>() == target) {
                return true;
            }
        }
        return false;
    }, timeoutMs);
}

void pumpEventsForMs(int ms) {
    QEventLoop loop;
    QTimer guard;
    guard.setSingleShot(true);
    QObject::connect(&guard, &QTimer::timeout, &loop, &QEventLoop::quit);
    guard.start(ms);
    loop.exec();
    QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

quint16 reserveClosedLoopbackPort() {
    QTcpServer probe;
    if (!probe.listen(QHostAddress::LocalHost, 0)) {
        return 0;
    }
    const quint16 port = probe.serverPort();
    probe.close();
    return port;
}

class TcpChannelTest : public Test {
protected:
    void SetUp() override {
        relay_ = std::make_unique<ChannelSignalRelay>();
        channel_ = std::make_unique<TcpChannel>();
    }

    void TearDown() override {
        relay_->detach();
        if (channel_) {
            channel_->close();
        }
        channel_.reset();
        relay_.reset();
    }

    std::unique_ptr<ChannelSignalRelay> relay_;
    std::unique_ptr<TcpChannel> channel_;
};

// Default construction state
TEST_F(TcpChannelTest, Construct_DefaultKindIsTcp) {
    EXPECT_EQ(channel_->kind(), ChannelKind::Tcp);
}

TEST_F(TcpChannelTest, Construct_DefaultStateIsClosed) {
    EXPECT_EQ(channel_->state(), ChannelState::Closed);
}

TEST_F(TcpChannelTest, Construct_DefaultNotOpen) {
    EXPECT_FALSE(channel_->isOpen());
}

// Endpoint configuration
TEST_F(TcpChannelTest, SetEndpoint_ValidIpPort_NoCrash) {
    EXPECT_NO_FATAL_FAILURE(channel_->setEndpoint(QStringLiteral("192.168.1.100"), 502));
}

TEST_F(TcpChannelTest, SetEndpoint_Localhost_NoCrash) {
    EXPECT_NO_FATAL_FAILURE(channel_->setEndpoint(QStringLiteral("127.0.0.1"), 1502));
}

// Write on closed channel
TEST_F(TcpChannelTest, Write_NoConnection_ReturnsFalse) {
    QByteArray testData = QByteArrayLiteral("test");
    bool wrote = channel_->write(testData);
    EXPECT_FALSE(wrote);
}

// Close from closed state
TEST_F(TcpChannelTest, Close_FromClosedState_NoCrash) {
    EXPECT_NO_FATAL_FAILURE(channel_->close());
    EXPECT_EQ(channel_->state(), ChannelState::Closed);
}

// Timeouts
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

// Stats
TEST_F(TcpChannelTest, Stats_InitialZero) {
    ChannelStats s = channel_->stats();
    EXPECT_EQ(s.bytesTx, 0);
    EXPECT_EQ(s.bytesRx, 0);
}

// Handlers — verify they can be set without crash
TEST_F(TcpChannelTest, SetReadHandler_NoCrash) {
    bool called = false;
    channel_->setReadHandler([&called](QByteArrayView) { called = true; });
    SUCCEED();
}

TEST_F(TcpChannelTest, SetErrorHandler_NoCrash) {
    bool called = false;
    channel_->setErrorHandler([&called](const io::ChannelError&) { called = true; });
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

// TCP endpoint validation
TEST_F(TcpChannelTest, TcpChannel_InvalidIpAddress_ReturnsError) {
    bool errorEmitted = false;
    channel_->setErrorHandler([&errorEmitted](const io::ChannelError&) {
        errorEmitted = true;
    });

    channel_->setEndpoint(QStringLiteral("not.an.ip"), 502);
    EXPECT_FALSE(channel_->open());

    EXPECT_TRUE(errorEmitted);
}

TEST_F(TcpChannelTest, TcpChannel_InvalidPort_ReturnsError) {
    bool errorEmitted = false;
    channel_->setErrorHandler([&errorEmitted](const io::ChannelError&) {
        errorEmitted = true;
    });

    channel_->setEndpoint(QStringLiteral("127.0.0.1"), 0);
    EXPECT_FALSE(channel_->open());

    EXPECT_TRUE(errorEmitted);
}

TEST_F(TcpChannelTest, TcpChannel_ValidParams_StartsOpeningWithoutImmediateError) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));

    relay_->attach(*channel_);
    QSignalSpy errorSpy(relay_.get(), &ChannelSignalRelay::errorOccurred);
    QSignalSpy stateSpy(relay_.get(), &ChannelSignalRelay::stateChanged);

    channel_->setEndpoint(QStringLiteral("127.0.0.1"),
                          static_cast<int>(server.serverPort()));
    EXPECT_TRUE(channel_->open());
    ASSERT_TRUE(waitForChannelState(*channel_, stateSpy, ChannelState::Open));
    EXPECT_EQ(errorSpy.count(), 0);
}

// Error is sticky: once the channel FSM is in Error, late socket
// state notifications must not repaint it into a healthy Closed/Open.
// Error is left only via open() (new attempt) or close() (user teardown).
TEST_F(TcpChannelTest, Error_LeavesOnlyViaOpenOrClose) {
    channel_->setEndpoint(QStringLiteral("not.an.ip"), 502);
    channel_->open();
    ASSERT_EQ(channel_->state(), ChannelState::Error);

    channel_->close();
    EXPECT_EQ(channel_->state(), ChannelState::Closed);

    channel_->setEndpoint(QStringLiteral("127.0.0.1"), 1502);
    channel_->open();
    EXPECT_NE(channel_->state(), ChannelState::Error);
}

TEST_F(TcpChannelTest, SocketError_StickyAgainstLateSocketStateChange) {
    relay_->attach(*channel_);
    QSignalSpy errorSpy(relay_.get(), &ChannelSignalRelay::errorOccurred);

    const quint16 refusedPort = reserveClosedLoopbackPort();
    ASSERT_NE(refusedPort, 0);

    io::Timeouts t;
    t.readMs = 1000;
    t.writeMs = 1000;
    channel_->setTimeouts(t);
    channel_->setEndpoint(QStringLiteral("127.0.0.1"),
                          static_cast<int>(refusedPort));

    ASSERT_TRUE(channel_->open());
    ASSERT_TRUE(waitForSpyCondition(errorSpy, [&errorSpy]() {
        return errorSpy.count() >= 1;
    }, 3000));
    ASSERT_EQ(channel_->state(), ChannelState::Error);

    pumpEventsForMs(60);
    EXPECT_EQ(channel_->state(), ChannelState::Error)
        << "late socket state change repainted sticky Error state";
}

// Close linger fallback: a close that cannot complete (kernel send
// buffer still holds unflushed data to a non-reading peer) must be
// force-completed after the linger timeout instead of staying in Closing.
TEST_F(TcpChannelTest, Close_StuckFlush_ForcedClosedAfterLinger) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));

    TcpChannel channel(100); // short linger for test velocity
    ChannelSignalRelay relay;
    relay.attach(channel);
    QSignalSpy stateSpy(&relay, &ChannelSignalRelay::stateChanged);

    channel.setEndpoint(QStringLiteral("127.0.0.1"),
                        static_cast<int>(server.serverPort()));
    ASSERT_TRUE(channel.open());
    ASSERT_TRUE(waitForChannelState(channel, stateSpy, ChannelState::Open));

    const QByteArray block(64 * 1024, 'x');
    for (int i = 0; i < 128; ++i) {
        channel.write(block);
    }

    stateSpy.clear();
    channel.close();

    ASSERT_TRUE(waitForChannelState(channel, stateSpy, ChannelState::Closed, 2000))
        << "close() stranded the channel in Closing (linger fallback failed)";
    EXPECT_EQ(channel.state(), ChannelState::Closed);
    relay.detach();
}

// T2-02: Closing during ConnectingState/HostLookupState must abort synchronously
// and reach Closed immediately without entering a stranded Closing state or
// firing a late queued ClosingState transition after Closed.
TEST_F(TcpChannelTest, Open_CloseDuringConnecting_AbortsSynchronouslyAndCloses) {
    const quint16 unusedPort = reserveClosedLoopbackPort();
    ASSERT_NE(unusedPort, 0);

    relay_->attach(*channel_);
    QSignalSpy stateSpy(relay_.get(), &ChannelSignalRelay::stateChanged);

    channel_->setEndpoint(QStringLiteral("127.0.0.1"), static_cast<int>(unusedPort));
    ASSERT_TRUE(channel_->open());
    ASSERT_EQ(channel_->state(), ChannelState::Opening);

    // Close immediately while connectToHost() is still in ConnectingState.
    channel_->close();
    EXPECT_EQ(channel_->state(), ChannelState::Closed)
        << "half-open close() should abort ConnectingState synchronously to Closed";

    pumpEventsForMs(50);
    EXPECT_EQ(channel_->state(), ChannelState::Closed);

    bool seenClosed = false;
    for (const auto& args : stateSpy) {
        const auto s = args.at(0).value<ChannelState>();
        if (s == ChannelState::Closed) {
            seenClosed = true;
        } else if (seenClosed) {
            EXPECT_NE(s, ChannelState::Closing)
                << "late queued stateChanged(ClosingState) reverted Closed -> Closing";
        }
    }
    EXPECT_TRUE(seenClosed);
}

// T2-02: close() followed immediately by open() in the same turn must not let
// the stale queued UnconnectedState from close() abort the new connect attempt.
TEST_F(TcpChannelTest, CloseThenImmediateOpen_OldQueuedUnconnectedDoesNotAbortNewConnect) {
    QTcpServer server1;
    QTcpServer server2;
    ASSERT_TRUE(server1.listen(QHostAddress::LocalHost, 0));
    ASSERT_TRUE(server2.listen(QHostAddress::LocalHost, 0));

    relay_->attach(*channel_);
    QSignalSpy stateSpy(relay_.get(), &ChannelSignalRelay::stateChanged);

    channel_->setEndpoint(QStringLiteral("127.0.0.1"), static_cast<int>(server1.serverPort()));
    ASSERT_TRUE(channel_->open());
    ASSERT_TRUE(waitForChannelState(*channel_, stateSpy, ChannelState::Open));

    // Close synchronously drops an idle connected socket to UnconnectedState while
    // leaving stateChanged(ClosingState/UnconnectedState) queued in the event loop.
    channel_->close();
    stateSpy.clear();
    channel_->setEndpoint(QStringLiteral("127.0.0.1"), static_cast<int>(server2.serverPort()));
    ASSERT_TRUE(channel_->open());

    ASSERT_TRUE(waitForChannelState(*channel_, stateSpy, ChannelState::Open, 3000));
    pumpEventsForMs(50);
    EXPECT_EQ(channel_->state(), ChannelState::Open)
        << "stale queued UnconnectedState aborted or closed the fresh connection";
}

// T2-02: Destroying a TcpChannel while Connecting or Connected (even without
// prior close() completion) must abort safely without hanging or crashing.
TEST_F(TcpChannelTest, Destruct_WhileConnectingOrConnected_NoCrash) {
    const quint16 unusedPort = reserveClosedLoopbackPort();
    ASSERT_NE(unusedPort, 0);

    {
        TcpChannel connectingChannel;
        connectingChannel.setEndpoint(QStringLiteral("127.0.0.1"), static_cast<int>(unusedPort));
        ASSERT_TRUE(connectingChannel.open());
    }

    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));
    {
        TcpChannel connectedChannel(500);
        ChannelSignalRelay localRelay;
        localRelay.attach(connectedChannel);
        QSignalSpy stateSpy(&localRelay, &ChannelSignalRelay::stateChanged);
        connectedChannel.setEndpoint(QStringLiteral("127.0.0.1"),
                                     static_cast<int>(server.serverPort()));
        ASSERT_TRUE(connectedChannel.open());
        ASSERT_TRUE(waitForChannelState(connectedChannel, stateSpy, ChannelState::Open));
        connectedChannel.write(QByteArrayLiteral("unflushed-payload"));
    }

    pumpEventsForMs(50);
    SUCCEED();
}

// T2-01: Cross-thread close() and setEndpoint() on a channel moved to an IO QThread
// must execute cleanly without data race or cross-thread QAbstractSocket access.
TEST_F(TcpChannelTest, CrossThread_CloseAndSetEndpoint_ReachesClosedAndReconnects) {
    QTcpServer server1;
    QTcpServer server2;
    ASSERT_TRUE(server1.listen(QHostAddress::LocalHost, 0));
    ASSERT_TRUE(server2.listen(QHostAddress::LocalHost, 0));

    auto ioThread = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    auto ioChannel = std::shared_ptr<TcpChannel>(
        new TcpChannel(100),
        [ioThread](TcpChannel* ch) {
            core::common::ThreadGuard::releaseChannel(ch, ioThread);
        });

    ChannelSignalRelay relay;
    relay.attach(*ioChannel);
    QSignalSpy stateSpy(&relay, &ChannelSignalRelay::stateChanged);
    QSignalSpy readSpy(&relay, &ChannelSignalRelay::dataReceived);

    ioChannel->moveToThread(ioThread.get());
    ioThread->start();

    ioChannel->setEndpoint(QStringLiteral("127.0.0.1"), static_cast<int>(server1.serverPort()));
    ASSERT_TRUE(ioChannel->open());
    ASSERT_TRUE(waitForChannelState(*ioChannel, stateSpy, ChannelState::Open));

    stateSpy.clear();
    ioChannel->close();
    ASSERT_TRUE(waitForChannelState(*ioChannel, stateSpy, ChannelState::Closed));

    // Reconfigure endpoint from main thread and reconnect to server2.
    stateSpy.clear();
    ioChannel->setEndpoint(QStringLiteral("127.0.0.1"), static_cast<int>(server2.serverPort()));
    ASSERT_TRUE(ioChannel->open());
    ASSERT_TRUE(waitForChannelState(*ioChannel, stateSpy, ChannelState::Open));

    ASSERT_TRUE(waitForSpyCondition(stateSpy, [&server2]() {
        return server2.hasPendingConnections();
    }, 1000) || server2.hasPendingConnections());
    QTcpSocket* peer = server2.nextPendingConnection();
    ASSERT_NE(peer, nullptr);
    peer->write(QByteArrayLiteral("ping"));
    peer->flush();

    ASSERT_TRUE(waitForSpyCondition(readSpy, [&readSpy]() {
        return readSpy.count() >= 1;
    }, 2000));
    EXPECT_EQ(readSpy.takeFirst().at(0).toByteArray(), QByteArrayLiteral("ping"));

    stateSpy.clear();
    ioChannel->close();
    ASSERT_TRUE(waitForChannelState(*ioChannel, stateSpy, ChannelState::Closed));
    relay.detach();
}

// T2-09: Peer disconnect/reset maps to ChannelErrorCode::ConnectionReset rather
// than generic ConnectionFailed.
TEST_F(TcpChannelTest, SocketError_PeerClose_MapsToConnectionReset) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));

    relay_->attach(*channel_);
    QSignalSpy stateSpy(relay_.get(), &ChannelSignalRelay::stateChanged);
    QSignalSpy errorSpy(relay_.get(), &ChannelSignalRelay::errorOccurred);
    QSignalSpy newConnSpy(&server, &QTcpServer::newConnection);

    channel_->setEndpoint(QStringLiteral("127.0.0.1"), static_cast<int>(server.serverPort()));
    ASSERT_TRUE(channel_->open());
    ASSERT_TRUE(waitForChannelState(*channel_, stateSpy, ChannelState::Open));
    ASSERT_TRUE(waitForSpyCondition(newConnSpy, [&server]() {
        return server.hasPendingConnections();
    }, 2000));

    QTcpSocket* serverPeer = server.nextPendingConnection();
    ASSERT_NE(serverPeer, nullptr);
    serverPeer->abort();

    ASSERT_TRUE(waitForSpyCondition(errorSpy, [&errorSpy]() {
        return errorSpy.count() >= 1;
    }, 2000));
    const auto args = errorSpy.takeFirst();
    EXPECT_EQ(args.at(0).value<ChannelErrorCode>(), ChannelErrorCode::ConnectionReset);
    EXPECT_EQ(channel_->state(), ChannelState::Error);
}

// T2-09: adoptSocket() configures LowDelayOption/KeepAliveOption and triggers
// reading of bytes already buffered before adoption.
TEST_F(TcpChannelTest, AdoptSocket_SetsOptionsAndReadsPrebufferedData) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));
    QSignalSpy newConnSpy(&server, &QTcpServer::newConnection);

    QTcpSocket client;
    QSignalSpy clientConnSpy(&client, &QTcpSocket::connected);
    client.connectToHost(QHostAddress::LocalHost, server.serverPort());
    ASSERT_TRUE(waitForSpyCondition(newConnSpy, [&server]() {
        return server.hasPendingConnections();
    }, 2000));
    ASSERT_TRUE(waitForSpyCondition(clientConnSpy, [&client]() {
        return client.state() == QAbstractSocket::ConnectedState;
    }, 2000));

    QTcpSocket* accepted = server.nextPendingConnection();
    ASSERT_NE(accepted, nullptr);
    QPointer<QTcpSocket> weakAccepted(accepted);

    QSignalSpy acceptedReadySpy(accepted, &QTcpSocket::readyRead);
    const QByteArray prebuffered = QByteArrayLiteral("hello-adopted");
    client.write(prebuffered);
    client.flush();

    ASSERT_TRUE(waitForSpyCondition(acceptedReadySpy, [accepted, &prebuffered]() {
        return accepted->bytesAvailable() >= prebuffered.size();
    }, 2000));

    relay_->attach(*channel_);
    QSignalSpy readSpy(relay_.get(), &ChannelSignalRelay::dataReceived);
    QSignalSpy drainedSpy(relay_.get(), &ChannelSignalRelay::writeDrained);

    ASSERT_TRUE(channel_->adoptSocket(accepted));
    EXPECT_EQ(channel_->state(), ChannelState::Open);
    ASSERT_NE(weakAccepted, nullptr);
    // POSIX/Darwin getsockopt() may return kernel bitmask flags (e.g. TF_NODELAY=4,
    // SO_KEEPALIVE=8 on macOS) rather than normalizing enabled booleans to 1.
    EXPECT_NE(weakAccepted->socketOption(QAbstractSocket::LowDelayOption).toInt(), 0);
    EXPECT_NE(weakAccepted->socketOption(QAbstractSocket::KeepAliveOption).toInt(), 0);

    ASSERT_TRUE(waitForSpyCondition(readSpy, [&readSpy]() {
        return readSpy.count() >= 1;
    }, 2000));
    EXPECT_EQ(readSpy.takeFirst().at(0).toByteArray(), prebuffered);
    EXPECT_EQ(channel_->stats().bytesRx, static_cast<quint64>(prebuffered.size()));

    // Verify TX roundtrip and writeDrained / bytesTx accounting on adopted socket.
    QSignalSpy clientReadSpy(&client, &QTcpSocket::readyRead);
    const QByteArray reply = QByteArrayLiteral("ack-adopted");
    EXPECT_TRUE(channel_->write(reply));
    ASSERT_TRUE(waitForSpyCondition(drainedSpy, [&drainedSpy]() {
        return drainedSpy.count() >= 1;
    }, 2000));
    ASSERT_TRUE(waitForSpyCondition(clientReadSpy, [&client, &reply]() {
        return client.bytesAvailable() >= reply.size();
    }, 2000));
    EXPECT_EQ(client.readAll(), reply);
    EXPECT_EQ(channel_->stats().bytesTx, static_cast<quint64>(reply.size()));
}

// T2-04: Entering ChannelState::Error via write timeout aborts the underlying
// socket, rejects subsequent writes, and recovers cleanly via a new open().
TEST_F(TcpChannelTest, WriteTimeout_AbortsSocketEntersErrorAndOpenRecovers) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));

    relay_->attach(*channel_);
    QSignalSpy stateSpy(relay_.get(), &ChannelSignalRelay::stateChanged);
    QSignalSpy errorSpy(relay_.get(), &ChannelSignalRelay::errorOccurred);
    QSignalSpy newConnSpy(&server, &QTcpServer::newConnection);

    io::Timeouts t;
    t.readMs = 1000;
    t.writeMs = 60;
    channel_->setTimeouts(t);
    channel_->setEndpoint(QStringLiteral("127.0.0.1"),
                          static_cast<int>(server.serverPort()));
    ASSERT_TRUE(channel_->open());
    ASSERT_TRUE(waitForChannelState(*channel_, stateSpy, ChannelState::Open));
    ASSERT_TRUE(waitForSpyCondition(newConnSpy, [&server]() {
        return server.hasPendingConnections();
    }, 2000));

    QTcpSocket* stalledPeer = server.nextPendingConnection();
    ASSERT_NE(stalledPeer, nullptr);
    stalledPeer->setReadBufferSize(1);
    QSignalSpy peerDisconnectedSpy(stalledPeer, &QTcpSocket::disconnected);

    // Push enough data that the stalled peer forces bytesToWrite() > 0 across
    // the 60ms write timeout window.
    const QByteArray burst(64 * 1024, 'z');
    for (int i = 0; i < 64 && channel_->state() == ChannelState::Open; ++i) {
        channel_->write(burst);
    }

    ASSERT_TRUE(waitForSpyCondition(errorSpy, [&errorSpy]() {
        return errorSpy.count() >= 1;
    }, 3000));
    EXPECT_EQ(errorSpy.takeFirst().at(0).value<ChannelErrorCode>(),
              ChannelErrorCode::Timeout);
    EXPECT_EQ(channel_->state(), ChannelState::Error);
    EXPECT_FALSE(channel_->write(QByteArrayLiteral("after-error")));

    // Clear the paused Qt read buffer so QAbstractSocket's read notifier wakes
    // up and observes the abort()/RST sent by TcpChannel::onChannelError().
    stalledPeer->setReadBufferSize(0);
    stalledPeer->readAll();
    ASSERT_TRUE(waitForSpyCondition(peerDisconnectedSpy, [stalledPeer, &peerDisconnectedSpy]() {
        if (stalledPeer->bytesAvailable() > 0) {
            stalledPeer->readAll();
        }
        return peerDisconnectedSpy.count() >= 1 ||
               stalledPeer->state() == QAbstractSocket::UnconnectedState;
    }, 2000)) << "underlying socket was not aborted on write-timeout Error";

    // Reopening after Error must establish a fresh connection instead of
    // short-circuiting on a stale connected handle.
    stateSpy.clear();
    newConnSpy.clear();
    ASSERT_TRUE(channel_->open());
    ASSERT_TRUE(waitForChannelState(*channel_, stateSpy, ChannelState::Open, 3000));
    ASSERT_TRUE(waitForSpyCondition(newConnSpy, [&server]() {
        return server.hasPendingConnections();
    }, 2000));
}

} // namespace

#include "TcpChannelTest.moc"
