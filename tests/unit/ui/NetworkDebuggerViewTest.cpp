// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "views/network/NetworkDebuggerView.h"
#include "widgets/TcpConnectionWidget.h"
#include "widgets/UdpConnectionWidget.h"
#include "widgets/GenericInputWidget.h"
#include "widgets/ByteMonitorWidget.h"
#include "widgets/ServerClientPanel.h"
#include "mocks/UiTestDoubles.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEventLoop>
#include <QListView>
#include <QNetworkDatagram>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUdpSocket>
#include <functional>
#include <memory>

namespace {

using Network = ui::views::network::NetworkDebuggerView;

bool await(const std::function<bool()>& ready) {
    if (ready()) return true;
    QEventLoop loop;
    QTimer probe;
    QTimer deadline;
    deadline.setSingleShot(true);
    QObject::connect(&probe, &QTimer::timeout, &loop, [&]() { if (ready()) loop.quit(); });
    QObject::connect(&deadline, &QTimer::timeout, &loop, &QEventLoop::quit);
    probe.start(5);
    deadline.start(3000);
    loop.exec();
    return ready();
}

bool monitorContains(Network& view, const QString& fragment) {
    auto* monitor = view.findChild<ui::widgets::ByteMonitorWidget*>();
    if (!monitor) return false;
    auto* list = monitor->findChild<QListView*>();
    if (!list || !list->model()) return false;
    for (int row = 0; row < list->model()->rowCount(); ++row) {
        if (list->model()->data(list->model()->index(row, 0)).toString().contains(fragment)) return true;
    }
    return false;
}

TEST(NetworkDebuggerLoopback, TcpClient_SendReceiveAndDisconnect) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    ui::widgets::TcpConnectionWidget* connection = nullptr;
    for (auto* candidate : view.findChildren<ui::widgets::TcpConnectionWidget*>()) {
        if (candidate->role() == ui::widgets::TcpRole::Client) connection = candidate;
    }
    auto* input = view.findChild<ui::widgets::GenericInputWidget*>();
    ASSERT_NE(connection, nullptr);
    ASSERT_NE(input, nullptr);
    connection->connectClicked(QStringLiteral("127.0.0.1"), server.serverPort());
    ASSERT_TRUE(await([&]() { return server.hasPendingConnections() && monitorContains(view, QStringLiteral("Connected")); }));
    std::unique_ptr<QTcpSocket> peer(server.nextPendingConnection());
    input->sendRequested(QByteArray::fromHex("12345678"));
    ASSERT_TRUE(await([&]() { return peer->bytesAvailable() >= 4; }));
    EXPECT_EQ(peer->readAll(), QByteArray::fromHex("12345678"));
    peer->write(QByteArray::fromHex("DEADBEEF"));
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("DE AD BE EF")); }));
    connection->disconnectClicked();
    EXPECT_TRUE(await([&]() { return peer->state() == QAbstractSocket::UnconnectedState; }));
}

TEST(NetworkDebuggerLoopback, TcpServer_SendReceiveAndStop) {
    QTcpServer reservation;
    ASSERT_TRUE(reservation.listen(QHostAddress::LocalHost, 0));
    const int port = reservation.serverPort();
    reservation.close();
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    view.findChild<QComboBox*>()->setCurrentIndex(1);
    ui::widgets::TcpConnectionWidget* connection = nullptr;
    for (auto* candidate : view.findChildren<ui::widgets::TcpConnectionWidget*>()) {
        if (candidate->role() == ui::widgets::TcpRole::Server) connection = candidate;
    }
    ASSERT_NE(connection, nullptr);
    connection->startListenClicked(QStringLiteral("127.0.0.1"), port);
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Listening")); }));
    QTcpSocket peer;
    peer.connectToHost(QHostAddress::LocalHost, port);
    auto* panel = view.findChild<ui::widgets::ServerClientPanel*>();
    ASSERT_NE(panel, nullptr);
    ASSERT_TRUE(await([&]() { return panel->hasClients() && peer.state() == QAbstractSocket::ConnectedState; }));
    panel->findChild<QCheckBox*>()->setChecked(true);
    view.findChild<ui::widgets::GenericInputWidget*>()->sendRequested(QByteArray::fromHex("12345678"));
    ASSERT_TRUE(await([&]() { return peer.bytesAvailable() >= 4; }));
    EXPECT_EQ(peer.readAll(), QByteArray::fromHex("12345678"));
    peer.write(QByteArray::fromHex("DEADBEEF"));
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("DE AD BE EF")); }));
    connection->stopListenClicked();
    EXPECT_TRUE(await([&]() { return peer.state() == QAbstractSocket::UnconnectedState; }));
}

TEST(NetworkDebuggerLoopback, Udp_SendReceiveAndUnbind) {
    QUdpSocket peer;
    ASSERT_TRUE(peer.bind(QHostAddress::LocalHost, 0));
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    view.findChild<QComboBox*>()->setCurrentIndex(2);
    auto* connection = view.findChild<ui::widgets::UdpConnectionWidget*>();
    ASSERT_NE(connection, nullptr);
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onBindClicked", Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, 0),
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, peer.localPort())));
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Connected")); }));
    view.findChild<ui::widgets::GenericInputWidget*>()->sendRequested(QByteArray::fromHex("12345678"));
    ASSERT_TRUE(await([&]() { return peer.hasPendingDatagrams(); }));
    const QNetworkDatagram received = peer.receiveDatagram();
    EXPECT_EQ(received.data(), QByteArray::fromHex("12345678"));
    peer.writeDatagram(QByteArray::fromHex("DEADBEEF"), received.senderAddress(), received.senderPort());
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("DE AD BE EF")); }));
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onUnbindClicked", Qt::DirectConnection));
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Closed")); }));
}

TEST(NetworkDebuggerLoopback, TcpClient_ProtocolLockedWhileActive) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    auto* protocolCombo = view.findChild<QComboBox*>();
    ASSERT_NE(protocolCombo, nullptr);
    EXPECT_TRUE(protocolCombo->isEnabled());

    ui::widgets::TcpConnectionWidget* connection = nullptr;
    for (auto* candidate : view.findChildren<ui::widgets::TcpConnectionWidget*>()) {
        if (candidate->role() == ui::widgets::TcpRole::Client) {
            connection = candidate;
        }
    }
    ASSERT_NE(connection, nullptr);
    connection->connectClicked(QStringLiteral("127.0.0.1"), server.serverPort());
    EXPECT_FALSE(protocolCombo->isEnabled());
    ASSERT_TRUE(await([&]() { return server.hasPendingConnections() && monitorContains(view, QStringLiteral("Connected")); }));
    EXPECT_FALSE(protocolCombo->isEnabled());

    connection->disconnectClicked();
    EXPECT_TRUE(await([&]() { return protocolCombo->isEnabled(); }));
}

TEST(NetworkDebuggerLoopback, TcpServer_AutoSendStopsWhenServerStops) {
    QTcpServer reservation;
    ASSERT_TRUE(reservation.listen(QHostAddress::LocalHost, 0));
    const int port = reservation.serverPort();
    reservation.close();

    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    view.findChild<QComboBox*>()->setCurrentIndex(1);

    ui::widgets::TcpConnectionWidget* connection = nullptr;
    for (auto* candidate : view.findChildren<ui::widgets::TcpConnectionWidget*>()) {
        if (candidate->role() == ui::widgets::TcpRole::Server) {
            connection = candidate;
        }
    }
    auto* input = view.findChild<ui::widgets::GenericInputWidget*>();
    ASSERT_NE(connection, nullptr);
    ASSERT_NE(input, nullptr);

    connection->startListenClicked(QStringLiteral("127.0.0.1"), port);
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Listening")); }));

    QTcpSocket peer;
    peer.connectToHost(QHostAddress::LocalHost, port);
    auto* panel = view.findChild<ui::widgets::ServerClientPanel*>();
    ASSERT_NE(panel, nullptr);
    ASSERT_TRUE(await([&]() { return panel->hasClients() && peer.state() == QAbstractSocket::ConnectedState; }));

    input->setAutoSendEnabled(true);
    EXPECT_TRUE(input->isAutoSendEnabled());

    connection->stopListenClicked();
    EXPECT_TRUE(await([&]() { return !input->isAutoSendEnabled(); }));
    EXPECT_TRUE(await([&]() { return peer.state() == QAbstractSocket::UnconnectedState; }));
}

TEST(NetworkDebuggerLoopback, Udp_RapidRebindResetsManualDisconnectAndGeneration) {
    QUdpSocket peer;
    ASSERT_TRUE(peer.bind(QHostAddress::LocalHost, 0));
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    view.findChild<QComboBox*>()->setCurrentIndex(2);

    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onBindClicked", Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, 0),
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, peer.localPort())));
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onUnbindClicked", Qt::DirectConnection));
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onBindClicked", Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, 0),
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, peer.localPort())));
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Connected")); }));

    view.findChild<ui::widgets::GenericInputWidget*>()->sendRequested(QByteArray::fromHex("CAFEBABE"));
    ASSERT_TRUE(await([&]() { return peer.hasPendingDatagrams(); }));
    EXPECT_EQ(peer.receiveDatagram().data(), QByteArray::fromHex("CAFEBABE"));

    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onUnbindClicked", Qt::DirectConnection));
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Closed")); }));
}

} // namespace
