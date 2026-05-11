#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../../ui/application/modbus/RequestSubmissionService.h"
#include "../../../ui/application/modbus/ModbusTypes.h"

using namespace testing;
using namespace ui::application::modbus;

namespace {

class RequestSubmissionServiceTest : public Test {
protected:
    void SetUp() override {
        service_ = std::make_unique<RequestSubmissionService>();
    }

    std::unique_ptr<RequestSubmissionService> service_;
};

TEST_F(RequestSubmissionServiceTest, BuildReadRequest_FC03_ReturnsOk) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 10;
    spec.slaveId = 1;

    auto result = service_->buildReadRequest(spec);

    EXPECT_TRUE(result.ok);
    EXPECT_GT(result.requestId, 0);
    EXPECT_EQ(static_cast<int>(result.pdu.functionCode()), 0x03);
}

TEST_F(RequestSubmissionServiceTest, BuildReadRequest_FC04_ReturnsOk) {
    PollSpec spec;
    spec.functionCode = 0x04;
    spec.startAddress = 100;
    spec.quantity = 5;

    auto result = service_->buildReadRequest(spec);

    EXPECT_TRUE(result.ok);
    EXPECT_EQ(static_cast<int>(result.pdu.functionCode()), 0x04);
}

TEST_F(RequestSubmissionServiceTest, BuildReadRequest_FC01_ReturnsOk) {
    PollSpec spec;
    spec.functionCode = 0x01;
    spec.startAddress = 0;
    spec.quantity = 8;

    auto result = service_->buildReadRequest(spec);

    EXPECT_TRUE(result.ok);
    EXPECT_EQ(static_cast<int>(result.pdu.functionCode()), 0x01);
}

TEST_F(RequestSubmissionServiceTest, BuildReadRequest_FC02_ReturnsOk) {
    PollSpec spec;
    spec.functionCode = 0x02;
    spec.startAddress = 10;
    spec.quantity = 16;

    auto result = service_->buildReadRequest(spec);

    EXPECT_TRUE(result.ok);
    EXPECT_EQ(static_cast<int>(result.pdu.functionCode()), 0x02);
}

TEST_F(RequestSubmissionServiceTest, BuildReadRequest_QuantityZero_ReturnsError) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 0;

    auto result = service_->buildReadRequest(spec);

    EXPECT_FALSE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildReadRequest_QuantityExceedsMax_ReturnsError) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 65535;

    auto result = service_->buildReadRequest(spec);

    EXPECT_FALSE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildReadRequest_DefaultKindIsRead) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 1;

    auto result = service_->buildReadRequest(spec);
    ASSERT_TRUE(result.ok);

    auto tracking = service_->lookupAndRemove(result.requestId);
    ASSERT_TRUE(tracking.has_value());
    EXPECT_EQ(tracking->kind, RequestKind::Read);
}

TEST_F(RequestSubmissionServiceTest, BuildReadRequest_PollKind_IsTrackedAsPoll) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 1;

    auto result = service_->buildReadRequest(spec, RequestKind::Poll);
    ASSERT_TRUE(result.ok);

    auto tracking = service_->lookupAndRemove(result.requestId);
    ASSERT_TRUE(tracking.has_value());
    EXPECT_EQ(tracking->kind, RequestKind::Poll);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_05_HexOn_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x05, 0, QStringLiteral("FF00"), QStringLiteral("Hex"), 1, 1);

    EXPECT_TRUE(result.ok);
    EXPECT_GT(result.requestId, 0);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_0F_Hex_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x0F, 0, QStringLiteral("FF03"), QStringLiteral("Hex"), 1, 10);

    EXPECT_TRUE(result.ok);
    EXPECT_GT(result.requestId, 0);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_05_HexOff_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x05, 0, QStringLiteral("0000"), QStringLiteral("Hex"), 1, 1);

    EXPECT_TRUE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_05_DecimalOne_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x05, 0, QStringLiteral("1"), QStringLiteral("Decimal"), 1, 1);

    EXPECT_TRUE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_05_DecimalZero_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x05, 0, QStringLiteral("0"), QStringLiteral("Decimal"), 1, 1);

    EXPECT_TRUE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_05_BinaryOne_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x05, 0, QStringLiteral("1"), QStringLiteral("Binary"), 1, 1);

    EXPECT_TRUE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_05_BinaryZero_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x05, 0, QStringLiteral("0"), QStringLiteral("Binary"), 1, 1);

    EXPECT_TRUE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_05_InvalidHex_ReturnsError) {
    auto result = service_->buildWriteRequest(0x05, 0, QStringLiteral("ABCD"), QStringLiteral("Hex"), 1, 1);

    EXPECT_FALSE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_06_Hex_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x06, 0, QStringLiteral("1234"), QStringLiteral("Hex"), 1, 1);

    EXPECT_TRUE(result.ok);
    EXPECT_GT(result.requestId, 0);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_06_Decimal_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x06, 0, QStringLiteral("65535"), QStringLiteral("Decimal"), 1, 1);

    EXPECT_TRUE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_06_DecimalOutOfRange_ReturnsError) {
    auto result = service_->buildWriteRequest(0x06, 0, QStringLiteral("70000"), QStringLiteral("Decimal"), 1, 1);

    EXPECT_FALSE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_06_BinaryNotSupported_ReturnsError) {
    auto result = service_->buildWriteRequest(0x06, 0, QStringLiteral("1010"), QStringLiteral("Binary"), 1, 1);

    EXPECT_FALSE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_0F_Binary_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x0F, 0, QStringLiteral("1010101010"), QStringLiteral("Binary"), 1, 10);

    EXPECT_TRUE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_0F_BinaryCountMismatch_ReturnsError) {
    auto result = service_->buildWriteRequest(0x0F, 0, QStringLiteral("101"), QStringLiteral("Binary"), 1, 10);

    EXPECT_FALSE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_10_Hex_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x10, 0, QStringLiteral("00010002"), QStringLiteral("Hex"), 1, 2);

    EXPECT_TRUE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_10_DecimalList_ReturnsOk) {
    auto result = service_->buildWriteRequest(0x10, 0, QStringLiteral("1,2,3"), QStringLiteral("Decimal"), 1, 3);

    EXPECT_TRUE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, BuildWriteRequest_10_EmptyValue_ReturnsError) {
    auto result = service_->buildWriteRequest(0x10, 0, QStringLiteral(""), QStringLiteral("Hex"), 1, 2);

    EXPECT_FALSE(result.ok);
}

TEST_F(RequestSubmissionServiceTest, RequestIdMonotonicallyIncreases) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 1;

    auto r1 = service_->buildReadRequest(spec);
    auto r2 = service_->buildReadRequest(spec);

    EXPECT_LT(r1.requestId, r2.requestId);
}

TEST_F(RequestSubmissionServiceTest, RequestIdWrapsAround) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 1;

    int lastId = 0;
    for (int i = 0; i < 5; ++i) {
        auto result = service_->buildReadRequest(spec);
        if (i == 0) {
            lastId = result.requestId;
            continue;
        }
        EXPECT_GT(result.requestId, lastId);
        lastId = result.requestId;
    }
}

TEST_F(RequestSubmissionServiceTest, LookupAndRemove_ExistingRequest_ReturnsInfo) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 42;
    spec.quantity = 1;

    auto result = service_->buildReadRequest(spec);
    ASSERT_TRUE(result.ok);

    auto tracking = service_->lookupAndRemove(result.requestId);
    ASSERT_TRUE(tracking.has_value());
    EXPECT_EQ(tracking->address, 42);
    EXPECT_EQ(tracking->kind, RequestKind::Read);
}

TEST_F(RequestSubmissionServiceTest, LookupAndRemove_NonExisting_ReturnsNullopt) {
    auto tracking = service_->lookupAndRemove(99999);
    EXPECT_FALSE(tracking.has_value());
}

TEST_F(RequestSubmissionServiceTest, LookupAndRemove_DoubleLookup_ReturnsNullopt) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 1;

    auto result = service_->buildReadRequest(spec);
    ASSERT_TRUE(result.ok);

    service_->lookupAndRemove(result.requestId);
    auto second = service_->lookupAndRemove(result.requestId);
    EXPECT_FALSE(second.has_value());
}

TEST_F(RequestSubmissionServiceTest, ClearAll_RemovesAllTrackedRequests) {
    PollSpec spec;
    spec.functionCode = 0x03;
    spec.startAddress = 0;
    spec.quantity = 1;

    service_->buildReadRequest(spec);
    service_->buildReadRequest(spec);
    service_->buildReadRequest(spec);

    service_->clearAll();

    EXPECT_FALSE(service_->lookupAndRemove(1).has_value());
    EXPECT_FALSE(service_->lookupAndRemove(2).has_value());
    EXPECT_FALSE(service_->lookupAndRemove(3).has_value());
}

TEST_F(RequestSubmissionServiceTest, ValidateRawData_EmptyData_ReturnsFalse) {
    QString error;
    bool ok = service_->validateRawData(QByteArray(), &error);
    EXPECT_FALSE(ok);
    EXPECT_FALSE(error.isEmpty());
}

TEST_F(RequestSubmissionServiceTest, ValidateRawData_NonEmptyData_ReturnsTrue) {
    bool ok = service_->validateRawData(QByteArray("\x01\x02\x03", 3));
    EXPECT_TRUE(ok);
}

} // namespace
