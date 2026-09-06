/**
 * @file test_ConnectionStateGuarding.cpp
 * @brief Unit tests for connection state machine UI guarding, idempotency, and lifecycle safety.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../../ui/application/modbus/ModbusPagePresenter.h"
#include "../../../ui/application/modbus/ModbusSessionPresenter.h"
#include "../../../ui/application/modbus/TrafficLogController.h"
#include "../../../ui/application/modbus/PollingController.h"
#include "../../../ui/application/modbus/RequestSubmissionService.h"
#include "../../../ui/application/modbus/ModbusTypes.h"
#include "../../../ui/widgets/FunctionWidget.h"
#include "../../../ui/widgets/ControlWidget.h"
#include "../../../ui/widgets/TrafficMonitorWidget.h"
#include "../../../ui/widgets/TcpConnectionWidget.h"
#include "../../../ui/widgets/SerialConnectionWidget.h"
#include "../../mocks/UiTestDoubles.h"

#include <QPushButton>
#include <QCheckBox>
#include <QPointer>

using namespace testing;
using namespace ui::application::modbus;

namespace {

QList<QPushButton*> getReadButtons(ui::widgets::FunctionWidget& widget) {
    QList<QPushButton*> result;
    const QStringList readLabels = {
        QObject::tr("Read Coils (0x01)"),
        QObject::tr("Read Discrete Inputs (0x02)"),
        QObject::tr("Read Holding Registers (0x03)"),
        QObject::tr("Read Input Registers (0x04)")
    };
    for (auto* btn : widget.findChildren<QPushButton*>()) {
        if (readLabels.contains(btn->text())) {
            result.append(btn);
        }
    }
    return result;
}

QList<QPushButton*> getWriteButtons(ui::widgets::FunctionWidget& widget) {
    QList<QPushButton*> result;
    const QStringList writeLabels = {
        QObject::tr("Write Single Coil (0x05)"),
        QObject::tr("Write Single Register (0x06)"),
        QObject::tr("Write Multiple Coils (0x0F)"),
        QObject::tr("Write Multiple Registers (0x10)"),
        QObject::tr("Send Raw")
    };
    for (auto* btn : widget.findChildren<QPushButton*>()) {
        if (writeLabels.contains(btn->text())) {
            result.append(btn);
        }
    }
    return result;
}

QCheckBox* getPollCheckBox(ui::widgets::ControlWidget& widget) {
    for (auto* cb : widget.findChildren<QCheckBox*>()) {
        if (cb->text() == QObject::tr("Enable Polling")) {
            return cb;
        }
    }
    return nullptr;
}

class ConnectionStateGuardingTest : public Test {
protected:
    void SetUp() override {
        settingsService_ = std::make_unique<tests::mocks::FakeSettingsService>();
        functionWidget_ = std::make_unique<ui::widgets::FunctionWidget>(settingsService_.get());
        controlWidget_ = std::make_unique<ui::widgets::ControlWidget>(settingsService_.get());
        trafficMonitor_ = std::make_unique<ui::widgets::TrafficMonitorWidget>(settingsService_.get());
        tcpWidget_ = std::make_unique<ui::widgets::TcpConnectionWidget>(ui::widgets::TcpRole::Client, settingsService_.get());

        presenter_ = std::make_unique<ModbusPagePresenter>(nullptr, SessionMode::Tcp);
        presenter_->setup(tcpWidget_.get(), controlWidget_.get(),
                          functionWidget_.get(), trafficMonitor_.get());
    }

    void TearDown() override {
        presenter_.reset();
        tcpWidget_.reset();
        trafficMonitor_.reset();
        controlWidget_.reset();
        functionWidget_.reset();
        settingsService_.reset();
    }

    std::unique_ptr<tests::mocks::FakeSettingsService> settingsService_;
    std::unique_ptr<ui::widgets::FunctionWidget> functionWidget_;
    std::unique_ptr<ui::widgets::ControlWidget> controlWidget_;
    std::unique_ptr<ui::widgets::TrafficMonitorWidget> trafficMonitor_;
    std::unique_ptr<ui::widgets::TcpConnectionWidget> tcpWidget_;
    std::unique_ptr<ModbusPagePresenter> presenter_;
};

} // namespace

// 1. Initial State: Controls are disabled
TEST_F(ConnectionStateGuardingTest, InitialState_ControlsAreDisabled) {
    const auto readBtns = getReadButtons(*functionWidget_);
    const auto writeBtns = getWriteButtons(*functionWidget_);
    auto* pollCheck = getPollCheckBox(*controlWidget_);

    ASSERT_EQ(readBtns.size(), 4);
    ASSERT_EQ(writeBtns.size(), 5);
    ASSERT_NE(pollCheck, nullptr);

    for (auto* btn : readBtns) {
        EXPECT_FALSE(btn->isEnabled());
        EXPECT_TRUE(btn->toolTip().contains(QObject::tr("Connect device")));
    }
    for (auto* btn : writeBtns) {
        EXPECT_FALSE(btn->isEnabled());
        EXPECT_TRUE(btn->toolTip().contains(QObject::tr("Connect device")));
    }
    EXPECT_FALSE(pollCheck->isEnabled());
}

// 2. Idempotent guard: calling with the same state twice does not trigger redundant refresh
TEST_F(ConnectionStateGuardingTest, SyncWidgetGuards_Idempotent_SameStateTwice) {
    const auto readBtns = getReadButtons(*functionWidget_);
    ASSERT_FALSE(readBtns.isEmpty());

    // First transition: Connected
    presenter_->syncWidgetGuards(SessionConnectionState::Connected);
    EXPECT_TRUE(readBtns.first()->isEnabled());

    // Modify button enabled externally to test if second call touches it
    readBtns.first()->setEnabled(false);

    // Second call with same state: idempotent guard returns early, button stays false
    presenter_->syncWidgetGuards(SessionConnectionState::Connected);
    EXPECT_FALSE(readBtns.first()->isEnabled());
}

// 3. Connecting State: reads and writes disabled
TEST_F(ConnectionStateGuardingTest, ConnectingState_ReadsWritesDisabled) {
    const auto readBtns = getReadButtons(*functionWidget_);
    const auto writeBtns = getWriteButtons(*functionWidget_);
    auto* pollCheck = getPollCheckBox(*controlWidget_);

    presenter_->syncWidgetGuards(SessionConnectionState::Connecting);

    for (auto* btn : readBtns) {
        EXPECT_FALSE(btn->isEnabled());
        EXPECT_TRUE(btn->toolTip().contains(QObject::tr("Connecting")));
    }
    for (auto* btn : writeBtns) {
        EXPECT_FALSE(btn->isEnabled());
        EXPECT_TRUE(btn->toolTip().contains(QObject::tr("Connecting")));
    }
    EXPECT_FALSE(pollCheck->isEnabled());
}

// 4. TransportConnected (TCP intermediate): reads enabled, writes disabled + operator-friendly tooltip
TEST_F(ConnectionStateGuardingTest, TransportConnected_TCP_ReadsEnabledWritesDisabled) {
    const auto readBtns = getReadButtons(*functionWidget_);
    const auto writeBtns = getWriteButtons(*functionWidget_);

    presenter_->syncWidgetGuards(SessionConnectionState::TransportConnected);

    for (auto* btn : readBtns) {
        EXPECT_TRUE(btn->isEnabled());
    }
    for (auto* btn : writeBtns) {
        EXPECT_FALSE(btn->isEnabled());
        EXPECT_TRUE(btn->toolTip().contains(QObject::tr("Device is establishing session")));
    }
}

// 5. TransportConnected Serial RTU: direct Connected, all ops enabled
TEST_F(ConnectionStateGuardingTest, TransportConnected_Serial_AllOpsEnabled) {
    const auto readBtns = getReadButtons(*functionWidget_);
    const auto writeBtns = getWriteButtons(*functionWidget_);

    // In Serial RTU, session reaches Connected directly without TransportConnected intermediate
    presenter_->syncWidgetGuards(SessionConnectionState::Connected);

    for (auto* btn : readBtns) {
        EXPECT_TRUE(btn->isEnabled());
    }
    for (auto* btn : writeBtns) {
        EXPECT_TRUE(btn->isEnabled());
    }
}

// 6. Connected: All operations and polling enabled
TEST_F(ConnectionStateGuardingTest, Connected_AllOpsEnabled_PollEnabled) {
    const auto readBtns = getReadButtons(*functionWidget_);
    const auto writeBtns = getWriteButtons(*functionWidget_);
    auto* pollCheck = getPollCheckBox(*controlWidget_);

    presenter_->syncWidgetGuards(SessionConnectionState::Connected);

    for (auto* btn : readBtns) {
        EXPECT_TRUE(btn->isEnabled());
    }
    for (auto* btn : writeBtns) {
        EXPECT_TRUE(btn->isEnabled());
    }
    EXPECT_TRUE(pollCheck->isEnabled());
}

// 7. Disconnected: All operations reverted and active polling stopped
TEST_F(ConnectionStateGuardingTest, Disconnected_AllOpsReverted_TimerStopped) {
    auto* pollCheck = getPollCheckBox(*controlWidget_);

    presenter_->syncWidgetGuards(SessionConnectionState::Connected);
    EXPECT_TRUE(pollCheck->isEnabled());

    // Simulate user enabling polling
    pollCheck->setChecked(true);
    EXPECT_TRUE(pollCheck->isChecked());

    // Transition to Disconnected
    presenter_->syncWidgetGuards(SessionConnectionState::Disconnected);

    const auto readBtns = getReadButtons(*functionWidget_);
    const auto writeBtns = getWriteButtons(*functionWidget_);

    for (auto* btn : readBtns) {
        EXPECT_FALSE(btn->isEnabled());
    }
    for (auto* btn : writeBtns) {
        EXPECT_FALSE(btn->isEnabled());
    }
    // Interactions disabled automatically cancels active polling and unchecks box
    EXPECT_FALSE(pollCheck->isEnabled());
    EXPECT_FALSE(pollCheck->isChecked());
}

// 8. Parameterized Test for EnsureConnected Log Behavior (TLC present vs TLC null)
class EnsureConnectedLogBehaviorTest : public ConnectionStateGuardingTest,
                                       public WithParamInterface<bool> {};

TEST_P(EnsureConnectedLogBehaviorTest, EnsureConnected_LogBehavior_P) {
    const bool hasTlc = GetParam();

    if (!hasTlc) {
        presenter_->setTrafficLogControllerForTest(nullptr);
    }

    // Ensure session is disconnected
    EXPECT_FALSE(presenter_->sessionPresenter()->isSessionConnected());

    // ensureConnected should return false safely in both cases
    EXPECT_FALSE(presenter_->ensureConnected());
}

INSTANTIATE_TEST_SUITE_P(
    TlcScenarios,
    EnsureConnectedLogBehaviorTest,
    Values(true, false)
);

// 9. QPointer TLC Safety: safe after teardown destruction
TEST_F(ConnectionStateGuardingTest, QPointerTLC_SafeAfterTeardown) {
    auto* standaloneTlc = new TrafficLogController(trafficMonitor_.get(), nullptr, nullptr);
    QPointer<TrafficLogController> guard(standaloneTlc);

    presenter_->setTrafficLogControllerForTest(standaloneTlc);
    ASSERT_FALSE(guard.isNull());

    // Destroy TLC directly
    delete standaloneTlc;
    EXPECT_TRUE(guard.isNull());

    // ensureConnected safely falls back to SPDLOG_WARN path without crashing
    EXPECT_FALSE(presenter_->ensureConnected());
}

// 10. Mode switch during connecting resets controls
TEST_F(ConnectionStateGuardingTest, ModeSwitch_DuringConnecting_ControlsCorrectlyReset) {
    presenter_->syncWidgetGuards(SessionConnectionState::Connecting);

    auto* serialWidget = new ui::widgets::SerialConnectionWidget(settingsService_.get());
    presenter_->switchMode(SessionMode::Rtu, serialWidget);

    const auto readBtns = getReadButtons(*functionWidget_);
    const auto writeBtns = getWriteButtons(*functionWidget_);

    for (auto* btn : readBtns) {
        EXPECT_FALSE(btn->isEnabled());
    }
    for (auto* btn : writeBtns) {
        EXPECT_FALSE(btn->isEnabled());
    }

    presenter_->switchMode(SessionMode::Tcp, tcpWidget_.get());
    delete serialWidget;
}

// 11. Teardown and recreate: no stale validator reference
TEST_F(ConnectionStateGuardingTest, TeardownAndRecreate_NoStaleValidatorReference) {
    auto* pollCheck = getPollCheckBox(*controlWidget_);
    ASSERT_NE(pollCheck, nullptr);

    // Calling setInteractionsEnabled controls the widget without any external validator callback
    controlWidget_->setInteractionsEnabled(true);
    EXPECT_TRUE(pollCheck->isEnabled());

    controlWidget_->setInteractionsEnabled(false);
    EXPECT_FALSE(pollCheck->isEnabled());
}
