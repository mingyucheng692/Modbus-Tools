// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include <QSignalSpy>
#include "application/ChannelSessionController.h"

using namespace ui::application;
using namespace io;

TEST(ChannelSessionControllerTest, InitialStateIsClosedAndSnapshotConsistent) {
    ChannelSessionController controller;
    EXPECT_EQ(controller.state(), ChannelState::Closed);
    EXPECT_EQ(controller.role(), ChannelRole::None);
    EXPECT_EQ(controller.generation(), 0u);

    const auto& snap = controller.currentSnapshot();
    EXPECT_EQ(snap.state, ChannelState::Closed);
    EXPECT_TRUE(snap.protocolSelectorEnabled);
    EXPECT_FALSE(snap.sendControlsEnabled);
    EXPECT_FALSE(snap.serialControlLinesEnabled);
    EXPECT_EQ(snap.sendDisabledTooltip, QStringLiteral("Connect first to send data"));
    EXPECT_EQ(snap.stateText, QStringLiteral("Disconnected"));
}

TEST(ChannelSessionControllerTest, OpenTransitionsImmediatelyToOpeningAndBumpsGeneration) {
    ChannelSessionController controller;
    QSignalSpy snapSpy(&controller, &ChannelSessionController::snapshotChanged);

    controller.openTcp(QStringLiteral("127.0.0.1"), 1234);
    EXPECT_EQ(controller.state(), ChannelState::Opening);
    EXPECT_EQ(controller.role(), ChannelRole::TcpClient);
    EXPECT_EQ(controller.generation(), 1u);

    ASSERT_GE(snapSpy.count(), 1);
    const auto& snap = controller.currentSnapshot();
    EXPECT_EQ(snap.state, ChannelState::Opening);
    EXPECT_FALSE(snap.protocolSelectorEnabled);
    EXPECT_FALSE(snap.sendControlsEnabled);
    EXPECT_EQ(snap.sendDisabledTooltip, QStringLiteral("Connecting..."));
    EXPECT_EQ(snap.stateText, QStringLiteral("Connecting..."));
}

TEST(ChannelSessionControllerTest, OpeningToOpenActivatesSendControlsAndClearsTooltip) {
    ChannelSessionController controller;
    controller.openTcp(QStringLiteral("127.0.0.1"), 1234);

    QSignalSpy snapSpy(&controller, &ChannelSessionController::snapshotChanged);
    controller.onWorkerStateChanged(ChannelState::Open, 1);

    EXPECT_EQ(controller.state(), ChannelState::Open);
    ASSERT_GE(snapSpy.count(), 1);
    const auto& snap = controller.currentSnapshot();
    EXPECT_TRUE(snap.sendControlsEnabled);
    EXPECT_TRUE(snap.sendDisabledTooltip.isEmpty());
    EXPECT_FALSE(snap.protocolSelectorEnabled);
    EXPECT_EQ(snap.stateText, QStringLiteral("Connected"));
}

TEST(ChannelSessionControllerTest, ActiveDisconnect_OpenToClosingToClosedSuppressesDisconnectAlert) {
    ChannelSessionController controller;
    controller.openTcp(QStringLiteral("127.0.0.1"), 1234);
    controller.onWorkerStateChanged(ChannelState::Open, 1);

    QSignalSpy unexpectedSpy(&controller, &ChannelSessionController::unexpectedDisconnectDetected);

    controller.closeChannel();
    EXPECT_EQ(controller.state(), ChannelState::Closing);
    EXPECT_EQ(controller.currentSnapshot().sendDisabledTooltip, QStringLiteral("Disconnecting..."));

    controller.onWorkerStateChanged(ChannelState::Closed, 1);
    EXPECT_EQ(controller.state(), ChannelState::Closed);
    EXPECT_EQ(unexpectedSpy.count(), 0);
}

TEST(ChannelSessionControllerTest, PassiveDisconnect_OpenToClosedEmitsUnexpectedDisconnect) {
    ChannelSessionController controller;
    controller.openTcp(QStringLiteral("127.0.0.1"), 1234);
    controller.onWorkerStateChanged(ChannelState::Open, 1);

    QSignalSpy unexpectedSpy(&controller, &ChannelSessionController::unexpectedDisconnectDetected);

    // Passive loss: peer closed connection directly while in Open state
    controller.onWorkerStateChanged(ChannelState::Closed, 1);
    EXPECT_EQ(controller.state(), ChannelState::Closed);
    EXPECT_EQ(unexpectedSpy.count(), 1);
}

TEST(ChannelSessionControllerTest, PassiveDisconnect_OpenToErrorEmitsUnexpectedDisconnect) {
    ChannelSessionController controller;
    controller.openTcp(QStringLiteral("127.0.0.1"), 1234);
    controller.onWorkerStateChanged(ChannelState::Open, 1);

    QSignalSpy unexpectedSpy(&controller, &ChannelSessionController::unexpectedDisconnectDetected);

    // Hardware/Socket error directly while in Open state
    controller.onWorkerStateChanged(ChannelState::Error, 1);
    EXPECT_EQ(controller.state(), ChannelState::Error);
    EXPECT_EQ(unexpectedSpy.count(), 1);
}

TEST(ChannelSessionControllerTest, FailedConnection_OpeningToErrorDoesNotEmitUnexpectedDisconnect) {
    ChannelSessionController controller;
    controller.openTcp(QStringLiteral("127.0.0.1"), 1234);

    QSignalSpy unexpectedSpy(&controller, &ChannelSessionController::unexpectedDisconnectDetected);

    // Initial connection attempt failed
    controller.onWorkerStateChanged(ChannelState::Error, 1);
    EXPECT_EQ(controller.state(), ChannelState::Error);
    EXPECT_EQ(unexpectedSpy.count(), 0);
}

TEST(ChannelSessionControllerTest, StaleGenerationDiscardsWorkerSignals) {
    ChannelSessionController controller;
    controller.openTcp(QStringLiteral("127.0.0.1"), 1234); // gen 1
    controller.closeChannel(); // cancels in-flight Opening -> gen 2
    controller.openTcp(QStringLiteral("127.0.0.1"), 5678); // gen 3
    EXPECT_EQ(controller.generation(), 3u);

    // Worker late callback from gen 1 or gen 2
    controller.onWorkerStateChanged(ChannelState::Open, 1);
    EXPECT_EQ(controller.state(), ChannelState::Opening); // Must NOT transition to Open!

    controller.onWorkerStateChanged(ChannelState::Open, 2);
    EXPECT_EQ(controller.state(), ChannelState::Opening); // Must NOT transition to Open!

    // Callback matching current generation 3
    controller.onWorkerStateChanged(ChannelState::Open, 3);
    EXPECT_EQ(controller.state(), ChannelState::Open);
}

TEST(ChannelSessionControllerTest, TcpServerTargetGatingAndTooltips) {
    ChannelSessionController controller;
    controller.openTcpServer(QStringLiteral("127.0.0.1"), 9999);
    controller.onWorkerStateChanged(ChannelState::Open, 1);

    EXPECT_EQ(controller.state(), ChannelState::Open);
    EXPECT_EQ(controller.currentSnapshot().stateText, QStringLiteral("Listening"));

    // Default: no target clients in unicast mode
    EXPECT_FALSE(controller.currentSnapshot().sendControlsEnabled);
    EXPECT_EQ(controller.currentSnapshot().sendDisabledTooltip, QStringLiteral("Select at least one client"));

    // Switch to broadcast mode with no clients
    controller.setServerTargetState(false, true);
    EXPECT_FALSE(controller.currentSnapshot().sendControlsEnabled);
    EXPECT_EQ(controller.currentSnapshot().sendDisabledTooltip, QStringLiteral("No connected clients"));

    // Clients connected and target exists
    controller.setServerTargetState(true, false);
    EXPECT_TRUE(controller.currentSnapshot().sendControlsEnabled);
    EXPECT_TRUE(controller.currentSnapshot().sendDisabledTooltip.isEmpty());
}

TEST(ChannelSessionControllerTest, AutoSendStopsWhenStateLeavesOpen) {
    ChannelSessionController controller;
    controller.openTcp(QStringLiteral("127.0.0.1"), 1234);
    controller.onWorkerStateChanged(ChannelState::Open, 1);

    QSignalSpy autoSendSpy(&controller, &ChannelSessionController::autoSendStateChanged);
    controller.setAutoSend(true, 50, []() { return QByteArrayLiteral("ping"); });
    EXPECT_TRUE(controller.isAutoSendActive());

    // Peer drops connection
    controller.onWorkerStateChanged(ChannelState::Closed, 1);
    EXPECT_FALSE(controller.isAutoSendActive());
    ASSERT_GE(autoSendSpy.count(), 1);
    EXPECT_FALSE(autoSendSpy.last().at(0).toBool());
}

TEST(ChannelSessionControllerTest, SerialRoleActivatesSerialControlLinesAndLazyWorkers) {
    ChannelSessionController controller;
    io::SerialConfig cfg;
    cfg.portName = QStringLiteral("COM1");

    EXPECT_FALSE(controller.hasOperationWorker());
    EXPECT_FALSE(controller.hasServerWorker());

    controller.openSerial(cfg);
    EXPECT_TRUE(controller.hasOperationWorker());
    EXPECT_FALSE(controller.hasServerWorker()); // Server worker must remain null!

    EXPECT_FALSE(controller.currentSnapshot().serialControlLinesEnabled);
    controller.onWorkerStateChanged(ChannelState::Open, 1);
    EXPECT_TRUE(controller.currentSnapshot().serialControlLinesEnabled);
}

TEST(ChannelSessionControllerTest, FormatHelpersProvideExpectedLabels) {
    EXPECT_EQ(ChannelSessionController::formatChannelState(ChannelState::Closed, ChannelRole::None), QStringLiteral("Disconnected"));
    EXPECT_EQ(ChannelSessionController::formatChannelState(ChannelState::Open, ChannelRole::TcpServer), QStringLiteral("Listening"));
    EXPECT_EQ(ChannelSessionController::formatChannelState(ChannelState::Open, ChannelRole::Udp), QStringLiteral("Bound"));
    EXPECT_EQ(ChannelSessionController::formatChannelState(ChannelState::Open, ChannelRole::Serial), QStringLiteral("Connected"));

    EXPECT_EQ(ChannelSessionController::formatChannelError(ChannelErrorCode::ConnectionFailed, QStringLiteral("refused")),
              QStringLiteral("Connection failed (refused)"));
    EXPECT_EQ(ChannelSessionController::formatChannelError(ChannelErrorCode::Timeout, QStringLiteral("")),
              QStringLiteral("Connection timeout"));
}
