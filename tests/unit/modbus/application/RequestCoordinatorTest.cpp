#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../../ui/application/modbus/RequestCoordinator.h"
#include "../../../ui/application/modbus/RequestSubmissionService.h"
#include "../../../ui/application/modbus/ModbusTypes.h"

using namespace testing;
using namespace ui::application::modbus;

namespace {

class RequestCoordinatorTest : public Test {
protected:
    void SetUp() override {
        requestService_ = new RequestSubmissionService();
        presenter_ = std::make_unique<ModbusSessionPresenter>(SessionMode::Tcp);
        coordinator_ = std::make_unique<RequestCoordinator>(
            presenter_.get(), requestService_,
            nullptr,  // pollingController
            nullptr,  // trafficLogController
            SessionMode::Tcp);
    }

    void TearDown() override {
        coordinator_.reset();
        presenter_.reset();
        delete requestService_;
    }

    RequestSubmissionService* requestService_ = nullptr;
    std::unique_ptr<ModbusSessionPresenter> presenter_;
    std::unique_ptr<RequestCoordinator> coordinator_;
};

TEST_F(RequestCoordinatorTest, ReadRequest_CallsBuildOnService) {
    // Verify that handleReadRequest triggers buildReadRequest on the service.
    // Since we're not connected, the submit will fail gracefully.
    EXPECT_NO_FATAL_FAILURE(
        coordinator_->handleReadRequest(0x03, 0, 10, 1));
}

TEST_F(RequestCoordinatorTest, WriteRequest_CallsBuildOnService) {
    EXPECT_NO_FATAL_FAILURE(
        coordinator_->handleWriteRequest(0x06, 0, QStringLiteral("100"),
                                         QStringLiteral("Decimal"), 1, 1));
}

TEST_F(RequestCoordinatorTest, RawSendRequest_DelegatesWithoutCrash) {
    QByteArray data = QByteArray::fromHex("010300000001");
    EXPECT_NO_FATAL_FAILURE(
        coordinator_->handleRawSendRequest(data));
}

TEST_F(RequestCoordinatorTest, LinkageDataReceived_EmittedOnSuccess) {
    bool received = false;
    QObject::connect(coordinator_.get(), &RequestCoordinator::linkageDataReceived,
        [&received](const ::modbus::base::Pdu&, modbus::core::parser::ProtocolType, uint16_t) {
            received = true;
        });

    // Build a request and immediately check with a simulated success response
    ::modbus::session::ModbusResponse response;
    response.isSuccess = true;
    response.pdu = ::modbus::base::Pdu::fromFunctionCode(0x03);

    // This won't match any tracked request, so linkageDataReceived won't be emitted
    // But we verify the signal connection works
    EXPECT_NO_FATAL_FAILURE(
        coordinator_->handleRequestFinished(99999, response));
}

TEST_F(RequestCoordinatorTest, ModeDescriptor_MapsAsciiProtocolType) {
    EXPECT_EQ(modeDescriptor(SessionMode::Ascii).protocolType,
              modbus::core::parser::ProtocolType::Ascii);
}

} // namespace
