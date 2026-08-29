/**
 * @file ServerChannelWorkerTest.cpp
 * @brief TCP server lifecycle tests (T3S.1 / T3S.2):
 *        - passive client loss must remove the client and free its slot
 *          (NEW-A regression: server-side ghost clients after peer RST)
 *        - a failed listen must emit a terminal Closed state instead of
 *          leaving the view stranded on "Connecting" (NEW-B)
 *        - closeClient (user-initiated) must not be double-reported
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>

#include <functional>

#include "../../../infra/io/ServerChannelWorker.h"
#include "../../../infra/io/TcpServerHandle.h"

using namespace testing;
using namespace io;

namespace {

/// Drives the event loop until @p predicate holds or the spin budget is
/// exhausted (queued socket signals need event-loop turns to be delivered).
bool spinUntil(const std::function<bool()>& predicate, int maxSpins = 300) {
    for (int i = 0; i < maxSpins; ++i) {
        if (predicate()) {
            return true;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    return predicate();
}

class TcpServerHandleTest : public Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(handle_.start(QStringLiteral("127.0.0.1"), 0, 1));
    }

    void TearDown() override {
        handle_.stop();
        for (int i = 0; i < 10; ++i) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
        }
        qDeleteAll(peers_);
        peers_.clear();
    }

    QTcpSocket* connectPeer() {
        auto* socket = new QTcpSocket();
        socket->connectToHost(QHostAddress::LocalHost, handle_.listenPort());
        return socket;
    }

    TcpServerHandle handle_;
    QList<QTcpSocket*> peers_;
};

// ---------------------------------------------------------------------------
// T3S.1 — passive client loss (NEW-A)
// ---------------------------------------------------------------------------

TEST_F(TcpServerHandleTest, PassiveLoss_AbortRemovesClientAndFreesSlot) {
    QSignalSpy connectedSpy(&handle_, &TcpServerHandle::clientConnected);
    QSignalSpy disconnectedSpy(&handle_, &TcpServerHandle::clientDisconnected);

    auto* peer = connectPeer();
    peers_.push_back(peer);
    ASSERT_TRUE(spinUntil([&connectedSpy]() { return connectedSpy.count() >= 1; }));
    ASSERT_EQ(handle_.clientCount(), 1);

    // Passive loss: hard abort emulates a peer RST / cable pull.
    peer->abort();
    ASSERT_TRUE(spinUntil([&disconnectedSpy, this]() {
        return disconnectedSpy.count() >= 1 && handle_.clientCount() == 0;
    }));
    EXPECT_EQ(handle_.clientCount(), 0) << "ghost client after passive loss";

    // The slot must be freed: with maxClients=1 a new peer is accepted.
    QSignalSpy reconnectedSpy(&handle_, &TcpServerHandle::clientConnected);
    auto* second = connectPeer();
    peers_.push_back(second);
    ASSERT_TRUE(spinUntil([&reconnectedSpy, this]() {
        return reconnectedSpy.count() >= 1 || handle_.clientCount() == 1;
    }));
    EXPECT_EQ(handle_.clientCount(), 1);
}

TEST_F(TcpServerHandleTest, PassiveLoss_GracefulDisconnectRemovesClient) {
    QSignalSpy connectedSpy(&handle_, &TcpServerHandle::clientConnected);
    QSignalSpy disconnectedSpy(&handle_, &TcpServerHandle::clientDisconnected);

    auto* peer = connectPeer();
    peers_.push_back(peer);
    ASSERT_TRUE(spinUntil([&connectedSpy]() { return connectedSpy.count() >= 1; }));
    ASSERT_EQ(handle_.clientCount(), 1);

    // Passive loss via orderly FIN.
    peer->disconnectFromHost();
    ASSERT_TRUE(spinUntil([&disconnectedSpy, this]() {
        return disconnectedSpy.count() >= 1 && handle_.clientCount() == 0;
    }));
}

// ---------------------------------------------------------------------------
// T3S.1 — user-initiated removal must emit exactly one clientDisconnected
// ---------------------------------------------------------------------------

TEST_F(TcpServerHandleTest, UserCloseClient_SingleDisconnectedEmission) {
    QSignalSpy connectedSpy(&handle_, &TcpServerHandle::clientConnected);
    QSignalSpy disconnectedSpy(&handle_, &TcpServerHandle::clientDisconnected);

    auto* peer = connectPeer();
    peers_.push_back(peer);
    ASSERT_TRUE(spinUntil([&connectedSpy]() { return connectedSpy.count() >= 1; }));
    ASSERT_EQ(handle_.clientCount(), 1);

    // Stop the server (user-initiated teardown of all clients). The state
    // handler installed for passive-loss detection must not re-enter and
    // double-report.
    handle_.stop();
    for (int i = 0; i < 20; ++i) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    EXPECT_EQ(handle_.clientCount(), 0);
    EXPECT_EQ(disconnectedSpy.count(), 1);
}

// ---------------------------------------------------------------------------
// T3S.2 — server state normalization (NEW-B)
// ---------------------------------------------------------------------------

TEST(ServerChannelWorkerTest, OpenTcpServer_Failure_EmitsErrorAndTerminalClosed) {
    // Occupy a port first so the worker's listen() deterministically fails.
    QTcpServer blocker;
    ASSERT_TRUE(blocker.listen(QHostAddress::LocalHost, 0));

    ServerChannelWorker worker;
    QSignalSpy errorSpy(&worker, &ServerChannelWorker::channelErrorOccurred);
    QSignalSpy stateSpy(&worker, &ServerChannelWorker::stateChanged);

    worker.openTcpServer(QStringLiteral("127.0.0.1"),
                         static_cast<int>(blocker.serverPort()), 0);

    ASSERT_GE(errorSpy.count(), 1);
    ASSERT_GE(stateSpy.count(), 1);
    // The terminal state must be Closed (the view falls back to its
    // disconnected display) — never a stranded Connecting.
    const QVariant last = stateSpy.last().at(0);
    EXPECT_EQ(last.value<ChannelState>(), ChannelState::Closed);
    // No Error state should be reported for a failed listen either: the
    // server never transitioned through a live session.
    for (const auto& args : stateSpy) {
        EXPECT_NE(args.at(0).value<ChannelState>(), ChannelState::Error);
    }
}

TEST(ServerChannelWorkerTest, OpenTcpServer_Success_EmitsOpen) {
    ServerChannelWorker worker;
    QSignalSpy stateSpy(&worker, &ServerChannelWorker::stateChanged);

    worker.openTcpServer(QStringLiteral("127.0.0.1"), 0, 0);
    ASSERT_GE(stateSpy.count(), 1);
    EXPECT_EQ(stateSpy.last().at(0).value<ChannelState>(), ChannelState::Open);

    QSignalSpy closedSpy(&worker, &ServerChannelWorker::stateChanged);
    worker.closeAllClients();
    ASSERT_GE(closedSpy.count(), 1);
    EXPECT_EQ(closedSpy.last().at(0).value<ChannelState>(), ChannelState::Closed);
}

} // namespace
