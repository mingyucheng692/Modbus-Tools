// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

// Acceptance for the TCP first-connect redesign: waitForChannelState() must
// run a nested event loop (not a sleep-spin) so the socket's connected()/
// errorOccurred() notifications can be delivered on the shared thread. These
// tests exercise real QAbstractSocket machinery against a localhost dummy
// server; the elapsed-time assertions discriminate against the old behavior,
// which idled out the full configured timeout on every first connect.

#include <gtest/gtest.h>

#include <QElapsedTimer>
#include <QHostAddress>
#include <QTcpServer>
#include <QThread>

#include "../../../infra/io/TcpChannel.h"
#include "../../helpers/ModbusTestHelpers.h"
#include "modbus/session/ModbusClient.h"
#include "modbus/transport/ModbusTcpTransport.h"

using modbus::base::ModbusConfig;
using modbus::base::ModbusMode;
using modbus::session::ModbusClient;

namespace {

struct ClientStack {
    std::shared_ptr<io::TcpChannel> channel;
    std::shared_ptr<ModbusClient> client;
};

// Builds a real client over a real TcpChannel. Caller thread == channel
// thread, matching the production topology where the worker drives the
// channel on the thread it owns.
ClientStack makeClient(const QString& host, quint16 port, int timeoutMs) {
    ClientStack stack;
    stack.channel = std::make_shared<io::TcpChannel>();
    stack.channel->setEndpoint(host, port);

    stack.client = std::make_shared<ModbusClient>(
        stack.channel, std::make_shared<modbus::transport::ModbusTcpTransport>());

    ModbusConfig cfg = modbus::test::MakeModbusConfig(ModbusMode::TCP, timeoutMs, 0);
    stack.client->setConfig(cfg);
    return stack;
}

TEST(TcpFirstConnectIntegrationTest, FirstConnect_SucceedsWellWithinTimeout) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));

    ClientStack stack = makeClient(QStringLiteral("127.0.0.1"),
                                   server.serverPort(), /*timeoutMs=*/1000);

    QElapsedTimer timer;
    timer.start();
    EXPECT_TRUE(stack.client->connect())
        << "first TCP connect failed: " << stack.client->lastChannelError().toStdString();
    // The old sleep-spin idled the full timeout (1000 ms) and failed; a real
    // localhost connect completes in single-digit milliseconds.
    EXPECT_LT(timer.elapsed(), 500)
        << "connect must be driven by the event loop, not idle to timeout";
    EXPECT_TRUE(stack.client->isConnected());

    stack.client->disconnect();
}

TEST(TcpFirstConnectIntegrationTest, ConnectRefused_FailsFastWithAccurateError) {
    // Grab a free port, then release it so nothing listens there.
    QTcpServer probe;
    ASSERT_TRUE(probe.listen(QHostAddress::LocalHost, 0));
    const quint16 port = probe.serverPort();
    probe.close();

    ClientStack stack = makeClient(QStringLiteral("127.0.0.1"), port, /*timeoutMs=*/1000);

    QElapsedTimer timer;
    timer.start();
    EXPECT_FALSE(stack.client->connect());
    // The refusal must surface via errorOccurred quickly on POSIX; the old
    // spin took the entire 1000 ms and reported a generic timeout.
    // Windows delivers the refused notification lazily (a raw QTcpSocket
    // probing a refused localhost port emits no errorOccurred for >2s under
    // pure event-driven pumping), so there the deterministic failure is the
    // channel's connect-timeout path; allow its runtime plus scheduler slack.
#if defined(Q_OS_WINDOWS)
    EXPECT_LT(timer.elapsed(), 1500)
        << "connect refusal must fail deterministically via the timeout path";
#else
    EXPECT_LT(timer.elapsed(), 500)
        << "connection refusal must fail fast via the error path";
#endif
    EXPECT_FALSE(stack.client->lastChannelError().isEmpty());
}

} // namespace
