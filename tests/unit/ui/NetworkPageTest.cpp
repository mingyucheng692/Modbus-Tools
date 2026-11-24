// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "views/network/NetworkPage.h"
#include "widgets/TcpConnectionWidget.h"
#include "widgets/UdpConnectionWidget.h"
#include "widgets/GenericInputWidget.h"
#include "widgets/ByteMonitorWidget.h"
#include "widgets/ServerClientPanel.h"
#include "widgets/CollapsibleSection.h"
#include "mocks/UiTestDoubles.h"
#include "core/common/SettingsKeys.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEventLoop>
#include <QListView>
#include <QNetworkDatagram>
#include <QScrollArea>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextEdit>
#include <QTimer>
#include <QUdpSocket>
#include <functional>
#include <memory>

namespace {

using Network = ui::views::network::NetworkPage;

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

// ---------------------------------------------------------------------------
// Unit Tests (Headless / Fast / In-Memory)
// ---------------------------------------------------------------------------

TEST(NetworkPageTest, Layout_720x480_NoOuterScrollAreaAndSectionsSync) {
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    view.resize(720, 480);
    EXPECT_GE(view.width(), 720);
    EXPECT_GE(view.height(), 480);

    // Page must be added directly to layout; zero outer/parent QScrollArea
    EXPECT_TRUE(view.findChildren<QScrollArea*>().isEmpty());

    auto* combo = view.findChild<QComboBox*>(QStringLiteral("networkProtocolCombo"));
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(combo->count(), 3);

    auto sections = view.findChildren<ui::widgets::CollapsibleSection*>();
    EXPECT_GE(sections.size(), 2);
}

TEST(NetworkPageTest, ProtocolChoiceRestoredFromSettings) {
    tests::mocks::FakeSettingsService settings;
    settings.setValue(QString::fromLatin1(core::common::settings_keys::kNetworkProtocol), 1); // TcpServer

    Network view(&settings);
    auto* combo = view.findChild<QComboBox*>(QStringLiteral("networkProtocolCombo"));
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(combo->currentIndex(), 1);

    combo->setCurrentIndex(2); // UDP
    EXPECT_EQ(settings.value(QString::fromLatin1(core::common::settings_keys::kNetworkProtocol)).toInt(), 2);
}

TEST(NetworkPageTest, DeclarativeFsmProjection_SnapshotReflectsToControls) {
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    auto* combo = view.findChild<QComboBox*>(QStringLiteral("networkProtocolCombo"));
    auto* input = view.findChild<ui::widgets::GenericInputWidget*>();
    ASSERT_NE(combo, nullptr);
    ASSERT_NE(input, nullptr);

    // Initial / Closed
    EXPECT_TRUE(combo->isEnabled());

    // Opening
    ui::application::SessionUiSnapshot openingSnap;
    openingSnap.state = io::ChannelState::Opening;
    openingSnap.stateText = QStringLiteral("Connecting...");
    openingSnap.protocolSelectorEnabled = false;
    openingSnap.sendControlsEnabled = false;
    openingSnap.sendDisabledTooltip = QStringLiteral("Connecting...");
    openingSnap.stateChanged = true;
    QMetaObject::invokeMethod(&view, "onSessionSnapshotChanged", Qt::DirectConnection,
                              Q_ARG(ui::application::SessionUiSnapshot, openingSnap));
    EXPECT_FALSE(combo->isEnabled());

    // Open
    ui::application::SessionUiSnapshot openSnap;
    openSnap.state = io::ChannelState::Open;
    openSnap.stateText = QStringLiteral("Connected");
    openSnap.protocolSelectorEnabled = false;
    openSnap.sendControlsEnabled = true;
    openSnap.sendDisabledTooltip = QString();
    openSnap.stateChanged = true;
    QMetaObject::invokeMethod(&view, "onSessionSnapshotChanged", Qt::DirectConnection,
                              Q_ARG(ui::application::SessionUiSnapshot, openSnap));
    EXPECT_FALSE(combo->isEnabled());

    // Closing
    ui::application::SessionUiSnapshot closingSnap;
    closingSnap.state = io::ChannelState::Closing;
    closingSnap.stateText = QStringLiteral("Closing");
    closingSnap.protocolSelectorEnabled = false;
    closingSnap.sendControlsEnabled = false;
    closingSnap.sendDisabledTooltip = QStringLiteral("Closing...");
    closingSnap.stateChanged = true;
    QMetaObject::invokeMethod(&view, "onSessionSnapshotChanged", Qt::DirectConnection,
                              Q_ARG(ui::application::SessionUiSnapshot, closingSnap));
    EXPECT_FALSE(combo->isEnabled());

    // Closed
    ui::application::SessionUiSnapshot closedSnap;
    closedSnap.state = io::ChannelState::Closed;
    closedSnap.stateText = QStringLiteral("Disconnected");
    closedSnap.protocolSelectorEnabled = true;
    closedSnap.sendControlsEnabled = false;
    closedSnap.sendDisabledTooltip = QStringLiteral("Disconnected");
    closedSnap.stateChanged = true;
    QMetaObject::invokeMethod(&view, "onSessionSnapshotChanged", Qt::DirectConnection,
                              Q_ARG(ui::application::SessionUiSnapshot, closedSnap));
    EXPECT_TRUE(combo->isEnabled());
}

TEST(NetworkPageTest, TcpServerTargetGatingAndTooltips) {
    tests::mocks::FakeSettingsService settings;
    settings.setValue(QString::fromLatin1(core::common::settings_keys::kNetworkProtocol), 1); // TcpServer
    Network view(&settings);

    // Server mode: no connected clients available
    ui::application::SessionUiSnapshot serverNoClientsSnap;
    serverNoClientsSnap.state = io::ChannelState::Open;
    serverNoClientsSnap.sendControlsEnabled = false;
    serverNoClientsSnap.sendDisabledTooltip = QStringLiteral("Server mode: no connected clients available");
    QMetaObject::invokeMethod(&view, "onSessionSnapshotChanged", Qt::DirectConnection,
                              Q_ARG(ui::application::SessionUiSnapshot, serverNoClientsSnap));

    // Server mode: select at least one client
    ui::application::SessionUiSnapshot serverUnselectedSnap;
    serverUnselectedSnap.state = io::ChannelState::Open;
    serverUnselectedSnap.sendControlsEnabled = false;
    serverUnselectedSnap.sendDisabledTooltip = QStringLiteral("Server mode: select at least one client");
    QMetaObject::invokeMethod(&view, "onSessionSnapshotChanged", Qt::DirectConnection,
                              Q_ARG(ui::application::SessionUiSnapshot, serverUnselectedSnap));

    // Server mode: ready with selection
    ui::application::SessionUiSnapshot serverReadySnap;
    serverReadySnap.state = io::ChannelState::Open;
    serverReadySnap.sendControlsEnabled = true;
    serverReadySnap.sendDisabledTooltip = QString();
    QMetaObject::invokeMethod(&view, "onSessionSnapshotChanged", Qt::DirectConnection,
                              Q_ARG(ui::application::SessionUiSnapshot, serverReadySnap));
}

TEST(NetworkPageTest, HexValidation_OddLengthAndInvalidCharsBlockSend) {
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    auto* inputWidget = view.findChild<ui::widgets::GenericInputWidget*>();
    ASSERT_NE(inputWidget, nullptr);
    auto* textEdit = inputWidget->findChild<QTextEdit*>();
    ASSERT_NE(textEdit, nullptr);

    // Enable send controls to test validation pipeline
    inputWidget->setSendControlsEnabled(true);

    QSignalSpy sendSpy(inputWidget, &ui::widgets::GenericInputWidget::sendRequested);
    QSignalSpy failSpy(inputWidget, &ui::widgets::GenericInputWidget::validationFailed);

    // Odd-length hex text
    textEdit->setPlainText(QStringLiteral("ABC"));
    QMetaObject::invokeMethod(inputWidget, "onSendClicked");
    EXPECT_EQ(sendSpy.count(), 0);
    EXPECT_GE(failSpy.count(), 1);

    // Non-hex characters
    failSpy.clear();
    textEdit->setPlainText(QStringLiteral("12 3Z"));
    QMetaObject::invokeMethod(inputWidget, "onSendClicked");
    EXPECT_EQ(sendSpy.count(), 0);
    EXPECT_GE(failSpy.count(), 1);

    // Valid hex
    textEdit->setPlainText(QStringLiteral("12 34"));
    QMetaObject::invokeMethod(inputWidget, "onSendClicked");
    ASSERT_EQ(sendSpy.count(), 1);
    EXPECT_EQ(sendSpy.at(0).at(0).toByteArray(), QByteArray::fromHex("1234"));
}

// ---------------------------------------------------------------------------
// Loopback Integration Tests (Requires Loopback Sockets)
// ---------------------------------------------------------------------------

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
    view.findChild<QComboBox*>(QStringLiteral("networkProtocolCombo"))->setCurrentIndex(1);
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
    view.findChild<QComboBox*>(QStringLiteral("networkProtocolCombo"))->setCurrentIndex(2);
    auto* connection = view.findChild<ui::widgets::UdpConnectionWidget*>();
    ASSERT_NE(connection, nullptr);
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onBindClicked", Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, 0),
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, peer.localPort())));
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Bound")); }));
    view.findChild<ui::widgets::GenericInputWidget*>()->sendRequested(QByteArray::fromHex("12345678"));
    ASSERT_TRUE(await([&]() { return peer.hasPendingDatagrams(); }));
    const QNetworkDatagram received = peer.receiveDatagram();
    EXPECT_EQ(received.data(), QByteArray::fromHex("12345678"));
    peer.writeDatagram(QByteArray::fromHex("DEADBEEF"), received.senderAddress(), received.senderPort());
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("DE AD BE EF")); }));
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onUnbindClicked", Qt::DirectConnection));
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Disconnected")); }));
}

TEST(NetworkDebuggerLoopback, TcpClient_ProtocolLockedWhileActive) {
    QTcpServer server;
    ASSERT_TRUE(server.listen(QHostAddress::LocalHost, 0));
    tests::mocks::FakeSettingsService settings;
    Network view(&settings);
    auto* protocolCombo = view.findChild<QComboBox*>(QStringLiteral("networkProtocolCombo"));
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
    view.findChild<QComboBox*>(QStringLiteral("networkProtocolCombo"))->setCurrentIndex(1);

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
    view.findChild<QComboBox*>(QStringLiteral("networkProtocolCombo"))->setCurrentIndex(2);

    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onBindClicked", Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, 0),
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, peer.localPort())));
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onUnbindClicked", Qt::DirectConnection));
    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onBindClicked", Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, 0),
        Q_ARG(QString, QStringLiteral("127.0.0.1")), Q_ARG(int, peer.localPort())));
    ASSERT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Bound")); }));

    view.findChild<ui::widgets::GenericInputWidget*>()->sendRequested(QByteArray::fromHex("CAFEBABE"));
    ASSERT_TRUE(await([&]() { return peer.hasPendingDatagrams(); }));
    EXPECT_EQ(peer.receiveDatagram().data(), QByteArray::fromHex("CAFEBABE"));

    ASSERT_TRUE(QMetaObject::invokeMethod(&view, "onUnbindClicked", Qt::DirectConnection));
    EXPECT_TRUE(await([&]() { return monitorContains(view, QStringLiteral("Disconnected")); }));
}

} // namespace
