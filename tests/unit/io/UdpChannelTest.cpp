// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QSignalSpy>
#include <QUdpSocket>

#include <functional>

#include "infra/io/UdpChannel.h"

using namespace io;

namespace {

class UdpSignalRelay : public QObject {
    Q_OBJECT
public:
    void attach(UdpChannel& channel) {
        channel_ = &channel;
        stateId_ = channel.addStateHandler([this](ChannelState s) {
            emit stateChanged(s);
        });
        channel.setErrorHandler([this](const ChannelError& err) {
            emit errorOccurred(err.code, err.message);
        });
        channel.setReadHandler([this](QByteArrayView data) {
            emit dataReceived(QByteArray(data.data(), data.size()));
        });
        channel.setMonitor([this](bool isTx, const QByteArray& data) {
            emit monitored(isTx, data);
        });
    }

    void detach() {
        if (!channel_) {
            return;
        }
        channel_->removeStateHandler(stateId_);
        stateId_ = 0;
        channel_->setErrorHandler(nullptr);
        channel_->setReadHandler(nullptr);
        channel_->setMonitor(nullptr);
        channel_ = nullptr;
    }

    ~UdpSignalRelay() override {
        detach();
    }

signals:
    void stateChanged(io::ChannelState state);
    void errorOccurred(io::ChannelErrorCode code, const QString& message);
    void dataReceived(const QByteArray& data);
    void monitored(bool isTx, const QByteArray& data);

private:
    UdpChannel* channel_ = nullptr;
    IChannel::HandlerId stateId_ = 0;
};

bool waitUntil(const std::function<bool()>& pred, int timeoutMs = 2000) {
    if (pred()) {
        return true;
    }
    QElapsedTimer timer;
    timer.start();
    while (!pred()) {
        if (timer.elapsed() >= timeoutMs) {
            return false;
        }
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    return true;
}

TEST(UdpChannelTest, BindConflict_EntersErrorStateAndEmitsError) {
    QUdpSocket blocker;
    ASSERT_TRUE(blocker.bind(QHostAddress::LocalHost, 0, QAbstractSocket::DontShareAddress));
    const quint16 occupiedPort = blocker.localPort();

    UdpChannel channel;
    UdpSignalRelay relay;
    relay.attach(channel);
    QSignalSpy errorSpy(&relay, &UdpSignalRelay::errorOccurred);
    QSignalSpy stateSpy(&relay, &UdpSignalRelay::stateChanged);

    channel.setBindEndpoint(QStringLiteral("127.0.0.1"), static_cast<int>(occupiedPort));
    EXPECT_FALSE(channel.open());
    EXPECT_EQ(channel.state(), ChannelState::Error);
    EXPECT_GE(errorSpy.count(), 1);

    // Sticky Error must survive queued UnconnectedState notifications.
    waitUntil([]() { return false; }, 40);
    EXPECT_EQ(channel.state(), ChannelState::Error);
    ASSERT_GE(stateSpy.count(), 1);
    EXPECT_EQ(stateSpy.last().at(0).value<ChannelState>(), ChannelState::Error);
}

TEST(UdpChannelTest, Write_WithoutRemotePeer_ReturnsFalseAndEmitsWriteFailed) {
    UdpChannel channel;
    UdpSignalRelay relay;
    relay.attach(channel);
    QSignalSpy errorSpy(&relay, &UdpSignalRelay::errorOccurred);
    QSignalSpy readSpy(&relay, &UdpSignalRelay::dataReceived);

    channel.setBindEndpoint(QStringLiteral("127.0.0.1"), 0);
    ASSERT_TRUE(channel.open());

    EXPECT_FALSE(channel.write(QByteArrayLiteral("no-peer")));
    ASSERT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).value<ChannelErrorCode>(), ChannelErrorCode::WriteFailed);

    waitUntil([]() { return false; }, 40);
    EXPECT_EQ(readSpy.count(), 0);
}

TEST(UdpChannelTest, Loopback_ZeroByteDatagramDoesNotHangAndFiltersRoguePeer) {
    QUdpSocket peer;
    ASSERT_TRUE(peer.bind(QHostAddress::LocalHost, 0));

    QUdpSocket rogue;
    ASSERT_TRUE(rogue.bind(QHostAddress::LocalHost, 0));

    UdpChannel channel;
    UdpSignalRelay relay;
    relay.attach(channel);
    QSignalSpy readSpy(&relay, &UdpSignalRelay::dataReceived);

    // Discover an ephemeral port via QUdpSocket first? Wait: UdpChannel binds port 0,
    // so send from channel -> peer to learn channel's ephemeral port cleanly.
    channel.setBindEndpoint(QStringLiteral("127.0.0.1"), 0);
    channel.setRemotePeer(QStringLiteral("127.0.0.1"), static_cast<int>(peer.localPort()));
    ASSERT_TRUE(channel.open());

    QHostAddress channelAddr;
    quint16 channelPort = 0;
    const QByteArray probe = QByteArrayLiteral("ping-udp");
    ASSERT_TRUE(channel.write(probe));
    ASSERT_TRUE(waitUntil([&peer]() { return peer.hasPendingDatagrams(); }));

    QByteArray peerRecv(static_cast<qsizetype>(peer.pendingDatagramSize()), Qt::Uninitialized);
    const qint64 readLen = peer.readDatagram(peerRecv.data(), peerRecv.size(), &channelAddr, &channelPort);
    ASSERT_EQ(readLen, probe.size());
    EXPECT_EQ(peerRecv, probe);
    EXPECT_EQ(channel.stats().bytesTx, static_cast<quint64>(probe.size()));

    // Rogue sender on another port must be filtered out.
    rogue.writeDatagram(QByteArrayLiteral("rogue"), QHostAddress::LocalHost, channelPort);
    waitUntil([]() { return false; }, 30);
    EXPECT_EQ(readSpy.count(), 0);

    // 0-byte datagram must be dequeued without wedging onReadyRead(), allowing
    // subsequent payload delivery.
    peer.writeDatagram(QByteArray(), QHostAddress::LocalHost, channelPort);
    const QByteArray reply = QByteArrayLiteral("pong-udp");
    peer.writeDatagram(reply, QHostAddress::LocalHost, channelPort);

    ASSERT_TRUE(waitUntil([&readSpy]() { return readSpy.count() >= 1; }));
    EXPECT_EQ(readSpy.count(), 1);
    EXPECT_EQ(readSpy.at(0).at(0).toByteArray(), reply);
    EXPECT_EQ(channel.stats().bytesRx, static_cast<quint64>(reply.size()));
}

TEST(UdpChannelTest, BroadcastTarget_AcceptsUnicastReplyOnRemotePort) {
    QUdpSocket responder;
    ASSERT_TRUE(responder.bind(QHostAddress::LocalHost, 0));

    QUdpSocket probeReceiver;
    ASSERT_TRUE(probeReceiver.bind(QHostAddress::LocalHost, 0));

    UdpChannel channel;
    UdpSignalRelay relay;
    relay.attach(channel);
    QSignalSpy readSpy(&relay, &UdpSignalRelay::dataReceived);

    // Discover the ephemeral local port bound by channel via a temporary loopback send.
    channel.setBindEndpoint(QStringLiteral("127.0.0.1"), 0);
    channel.setRemotePeer(QStringLiteral("127.0.0.1"), static_cast<int>(probeReceiver.localPort()));
    ASSERT_TRUE(channel.open());
    ASSERT_TRUE(channel.write(QByteArrayLiteral("hi")));
    ASSERT_TRUE(waitUntil([&probeReceiver]() { return probeReceiver.hasPendingDatagrams(); }));
    QHostAddress boundAddr;
    quint16 boundPort = 0;
    QByteArray tmp(static_cast<qsizetype>(probeReceiver.pendingDatagramSize()), Qt::Uninitialized);
    probeReceiver.readDatagram(tmp.data(), tmp.size(), &boundAddr, &boundPort);

    // Switch remote peer to broadcast address at responder's port. Unicast reply
    // from 127.0.0.1:<responderPort> must be delivered rather than filtered.
    channel.setRemotePeer(QStringLiteral("255.255.255.255"), static_cast<int>(responder.localPort()));
    const QByteArray bcastReply = QByteArrayLiteral("bcast-ack");
    responder.writeDatagram(bcastReply, QHostAddress::LocalHost, boundPort);

    ASSERT_TRUE(waitUntil([&readSpy]() { return readSpy.count() >= 1; }));
    EXPECT_EQ(readSpy.at(0).at(0).toByteArray(), bcastReply);
}

} // namespace

#include "UdpChannelTest.moc"
