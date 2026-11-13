// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "application/AnalyzerLinkCoordinator.h"
#include "views/analyzer/FrameAnalyzerView.h"
#include "views/modbus/ModbusPage.h"
#include "widgets/modbus/ModbusTelemetryWidget.h"
#include "mocks/UiTestDoubles.h"

#include <QTableWidget>

namespace {

using Coordinator = ui::application::AnalyzerLinkCoordinator;
using LinkState = Coordinator::LinkState;

TEST(AnalyzerLinkCoordinatorTest, BindNullPointers_IsSafeNoOp) {
    Coordinator coordinator;
    coordinator.bind(nullptr, nullptr);
    EXPECT_EQ(coordinator.state(), LinkState::Idle);
}

TEST(AnalyzerLinkCoordinatorTest, ToggleLiveForwardsPduAndPauseBuffersThenReplays) {
    tests::mocks::FakeSettingsService settings;
    ui::views::modbus::ModbusPage modbusPage(&settings);
    ui::views::analyzer::FrameAnalyzerView analyzerView(&settings);
    Coordinator coordinator;
    coordinator.bind(&modbusPage, &analyzerView);

    EXPECT_EQ(coordinator.state(), LinkState::Idle);

    // Activate linkage from ModbusPage
    emit modbusPage.linkageToggled(true);
    EXPECT_EQ(coordinator.state(), LinkState::Live);
    EXPECT_TRUE(modbusPage.telemetryWidget()->isLinkageActive());

    // Forward 1 register PDU in Live state -> analyzer decoded table gets 1 row
    const ::modbus::base::Pdu livePdu(
        ::modbus::base::FunctionCode::ReadHoldingRegisters,
        QByteArray::fromHex("020064"));
    emit modbusPage.linkageDataReceived(livePdu, ::modbus::parser::ProtocolType::Rtu, 10);
    auto* table = analyzerView.findChild<QTableWidget*>();
    ASSERT_NE(table, nullptr);
    EXPECT_EQ(table->rowCount(), 1);

    // Pause linkage from FrameAnalyzerView -> incoming 2-register PDU is buffered, not applied yet
    emit analyzerView.linkagePauseToggled(true);
    EXPECT_EQ(coordinator.state(), LinkState::Paused);
    const ::modbus::base::Pdu bufferedPdu(
        ::modbus::base::FunctionCode::ReadHoldingRegisters,
        QByteArray::fromHex("04006400C8"));
    emit modbusPage.linkageDataReceived(bufferedPdu, ::modbus::parser::ProtocolType::Rtu, 20);
    EXPECT_EQ(table->rowCount(), 1);

    // Resume linkage -> buffered 2-register PDU is replayed immediately
    emit analyzerView.linkagePauseToggled(false);
    EXPECT_EQ(coordinator.state(), LinkState::Live);
    EXPECT_EQ(table->rowCount(), 2);
}

TEST(AnalyzerLinkCoordinatorTest, StopRequestedAndSourceDisconnectedResetToIdle) {
    tests::mocks::FakeSettingsService settings;
    ui::views::modbus::ModbusPage modbusPage(&settings);
    ui::views::analyzer::FrameAnalyzerView analyzerView(&settings);
    Coordinator coordinator;
    coordinator.bind(&modbusPage, &analyzerView);

    emit modbusPage.linkageToggled(true);
    ASSERT_EQ(coordinator.state(), LinkState::Live);

    emit analyzerView.linkageStopRequested();
    EXPECT_EQ(coordinator.state(), LinkState::Idle);
    EXPECT_FALSE(modbusPage.telemetryWidget()->isLinkageActive());

    // Re-enable and verify source disconnect also transitions back to Idle
    emit modbusPage.linkageToggled(true);
    ASSERT_EQ(coordinator.state(), LinkState::Live);

    emit modbusPage.linkageSourceDisconnected();
    EXPECT_EQ(coordinator.state(), LinkState::Idle);
    EXPECT_FALSE(modbusPage.telemetryWidget()->isLinkageActive());
}

} // namespace
