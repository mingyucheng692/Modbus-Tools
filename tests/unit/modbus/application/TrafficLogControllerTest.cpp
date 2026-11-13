// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../../ui/application/modbus/TrafficLogController.h"
#include "../../../ui/application/modbus/PollingController.h"
#include "../../../ui/application/modbus/RequestSubmissionService.h"
#include "../../../ui/application/modbus/ModbusTypes.h"
#include "Config.h"
#include <QSignalSpy>

using namespace testing;
using namespace ui::application::modbus;

namespace {

class TrafficLogControllerTest : public Test {
protected:
    void SetUp() override {
        requestService_ = std::make_unique<RequestSubmissionService>();
        pollingController_ = std::make_unique<PollingController>(requestService_.get());
        controller_ = std::make_unique<TrafficLogController>(
            pollingController_.get());
    }

    std::unique_ptr<RequestSubmissionService> requestService_;
    std::unique_ptr<PollingController> pollingController_;
    std::unique_ptr<TrafficLogController> controller_;
};

TEST_F(TrafficLogControllerTest, LogConnectionInfo_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logConnectionInfo(QStringLiteral("Connected")));
}

TEST_F(TrafficLogControllerTest, LogRawFrame_TxDirection_DoesNotCrash) {
    QByteArray data("\x01\x02\x03", 3);

    EXPECT_NO_THROW(controller_->logRawFrame(ui::common::TrafficDirection::Tx, data));
}

TEST_F(TrafficLogControllerTest, LogRawFrame_RxDirection_DoesNotCrash) {
    QByteArray data("\xAA\xBB", 2);

    EXPECT_NO_THROW(controller_->logRawFrame(ui::common::TrafficDirection::Rx, data));
}

TEST_F(TrafficLogControllerTest, LogReadSuccess_NoRetry_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logReadSuccess(0));
}

TEST_F(TrafficLogControllerTest, LogReadSuccess_WithRetry_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logReadSuccess(3));
}

TEST_F(TrafficLogControllerTest, LogWriteSuccess_NoRetry_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logWriteSuccess(0));
}

TEST_F(TrafficLogControllerTest, LogBroadcastWriteSuccess_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logBroadcastWriteSuccess(0));
}

TEST_F(TrafficLogControllerTest, LogRequestError_NoRetry_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logRequestError(QStringLiteral("CRC error"), 0));
}

TEST_F(TrafficLogControllerTest, LogRequestError_WithRetry_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logRequestError(QStringLiteral("timeout"), 2));
}

TEST_F(TrafficLogControllerTest, LogSendingReadRequest_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logSendingReadRequest(0x03, 100, 10, 1));
}

TEST_F(TrafficLogControllerTest, LogSendingWriteRequest_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logSendingWriteRequest(0x06, 5, QStringLiteral("ABCD"), 1));
}

TEST_F(TrafficLogControllerTest, LogSendingRawData_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logSendingRawData(QByteArray("\xAB\xCD", 2)));
}

TEST_F(TrafficLogControllerTest, LogError_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logError(QStringLiteral("test error")));
}

TEST_F(TrafficLogControllerTest, LogWarning_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logWarning(QStringLiteral("warning msg")));
}

TEST_F(TrafficLogControllerTest, LogInfo_DoesNotCrash) {
    EXPECT_NO_THROW(controller_->logInfo(QStringLiteral("info msg")));
}

TEST_F(TrafficLogControllerTest, LogPollSummary_DoesNotCrash) {
    PollSummary summary;
    summary.functionCode = 0x03;
    summary.address = 100;
    summary.quantity = 10;
    summary.slaveId = 1;
    summary.successCount = 50;
    summary.errorCount = 2;
    summary.retryCount = 3;
    summary.avgRttMs = 15;

    EXPECT_NO_THROW(controller_->logPollSummary(summary));
}

TEST_F(TrafficLogControllerTest, SetPollingController_UpdatesReference) {
    auto newPollingController = std::make_unique<PollingController>(requestService_.get());

    EXPECT_NO_THROW(controller_->setPollingController(newPollingController.get()));
}

TEST_F(TrafficLogControllerTest, PublishEvent_EmitsTrafficEventSignal) {
    qRegisterMetaType<ui::common::TrafficEvent>();
    QSignalSpy spy(controller_.get(), &TrafficLogController::trafficEvent);
    controller_->logConnectionInfo(QStringLiteral("Connected successfully"));
    ASSERT_EQ(spy.count(), 1);
    auto ev = spy.first().at(0).value<ui::common::TrafficEvent>();
    EXPECT_EQ(ev.summary, QStringLiteral("Connected successfully"));
}

TEST_F(TrafficLogControllerTest, LogRawFrame_SuppressedBetweenPollTicksWhenRawDisabled_AndSampledWhenRawEnabled) {
    pollingController_->handleSessionConnected();
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 4;
    spec.slaveId = 1;
    spec.intervalMs = 500;
    pollingController_->handlePollRequest(spec);
    pollingController_->handleResponse(true, 5, 0, QString());

    ASSERT_TRUE(pollingController_->isPollingActive());
    ASSERT_TRUE(pollingController_->isSuppressingTrafficLog());

    QSignalSpy spy(controller_.get(), &TrafficLogController::trafficEvent);
    controller_->setRawFramesEnabled(false);
    controller_->logRawFrame(ui::common::TrafficDirection::Tx, QByteArray::fromHex("010300000004"));
    controller_->logRawFrame(ui::common::TrafficDirection::Rx, QByteArray::fromHex("0103080000"));
    EXPECT_EQ(spy.count(), 0);

    controller_->setRawFramesEnabled(true);
    for (int i = 0; i < config::Ui::kRawFrameSampleRate; ++i) {
        controller_->logRawFrame(ui::common::TrafficDirection::Tx, QByteArray::fromHex("010300000004"));
    }
    ASSERT_EQ(spy.count(), 1);
    const auto sampled = spy.first().first().value<ui::common::TrafficEvent>();
    EXPECT_TRUE(sampled.isPoll);

    PollSummary summary;
    summary.successCount = 1;
    summary.avgRttMs = 5;
    controller_->logPollSummary(summary);
    ASSERT_EQ(spy.count(), 2);
    const auto summaryEv = spy.last().first().value<ui::common::TrafficEvent>();
    EXPECT_TRUE(summaryEv.summary.contains(QStringLiteral("raw frames: 1 shown")));
}

} // namespace
