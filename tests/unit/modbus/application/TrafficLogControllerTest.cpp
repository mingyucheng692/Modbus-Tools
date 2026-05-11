#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../../ui/application/modbus/TrafficLogController.h"
#include "../../../ui/application/modbus/PollingController.h"
#include "../../../ui/application/modbus/RequestSubmissionService.h"
#include "../../../ui/application/modbus/ModbusTypes.h"
#include "../../../ui/widgets/TrafficMonitorWidget.h"

using namespace testing;
using namespace ui::application::modbus;

namespace {

class TrafficLogControllerTest : public Test {
protected:
    void SetUp() override {
        requestService_ = std::make_unique<RequestSubmissionService>();
        pollingController_ = std::make_unique<PollingController>(requestService_.get());
        monitor_ = new ui::widgets::TrafficMonitorWidget(nullptr);
        controller_ = std::make_unique<TrafficLogController>(
            monitor_, pollingController_.get());
    }

    std::unique_ptr<RequestSubmissionService> requestService_;
    std::unique_ptr<PollingController> pollingController_;
    ui::widgets::TrafficMonitorWidget* monitor_;
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

} // namespace
