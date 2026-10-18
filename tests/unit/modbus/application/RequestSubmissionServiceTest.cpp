// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../../ui/application/modbus/RequestSubmissionService.h"
#include "../../../ui/application/modbus/ModbusTypes.h"
#include "../../../ui/application/modbus/ModbusExecuteSpec.h"

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

TEST_F(RequestSubmissionServiceTest, ModeDescriptor_MapsAsciiProtocolType) {
    EXPECT_EQ(modeDescriptor(SessionMode::Ascii).protocolType,
              modbus::parser::ProtocolType::Ascii);
}

TEST_F(RequestSubmissionServiceTest, ValidateSpec_Gate1_BroadcastDefense) {
    ModbusExecuteSpec spec;
    spec.slaveId = 0; // Broadcast
    spec.functionCode = 0x03; // Read Holding
    spec.startAddress = 0;
    spec.quantity = 10;

    // Broadcast read is forbidden
    auto res = service_->validateSpec(spec, SessionMode::Rtu);
    EXPECT_FALSE(res.isValid);
    EXPECT_TRUE(res.errorMessage.contains("Broadcast read"));

    res = service_->validateSpec(spec, SessionMode::Tcp);
    EXPECT_FALSE(res.isValid);

    // Broadcast write is allowed
    spec.functionCode = 0x06;
    spec.quantity = 1;
    spec.writeData = QByteArray("\x00\x05", 2);
    res = service_->validateSpec(spec, SessionMode::Rtu);
    EXPECT_TRUE(res.isValid);

    spec.functionCode = 0x10;
    spec.quantity = 2;
    spec.writeData = QByteArray("\x00\x01\x00\x02", 4);
    res = service_->validateSpec(spec, SessionMode::Rtu);
    EXPECT_TRUE(res.isValid);

    // Slave ID > 247 forbidden
    spec.slaveId = 248;
    res = service_->validateSpec(spec, SessionMode::Rtu);
    EXPECT_FALSE(res.isValid);
    EXPECT_TRUE(res.errorMessage.contains("Slave ID"));
}

TEST_F(RequestSubmissionServiceTest, ValidateSpec_Gate2_3_QuantityLimits) {
    ModbusExecuteSpec spec;
    spec.slaveId = 1;
    spec.functionCode = 0x03;
    spec.startAddress = 0;

    // Zero quantity
    spec.quantity = 0;
    EXPECT_FALSE(service_->validateSpec(spec).isValid);

    // Max boundary for FC 03 (125)
    spec.quantity = 125;
    EXPECT_TRUE(service_->validateSpec(spec).isValid);

    // Exceeds max for FC 03 (126)
    spec.quantity = 126;
    EXPECT_FALSE(service_->validateSpec(spec).isValid);

    // FC 01 max (2000)
    spec.functionCode = 0x01;
    spec.quantity = 2000;
    EXPECT_TRUE(service_->validateSpec(spec).isValid);

    spec.quantity = 2001;
    EXPECT_FALSE(service_->validateSpec(spec).isValid);

    // Single write must be exactly quantity 1
    spec.functionCode = 0x05;
    spec.quantity = 2;
    spec.writeData = QByteArray("\xFF\x00", 2);
    EXPECT_FALSE(service_->validateSpec(spec).isValid);
}

TEST_F(RequestSubmissionServiceTest, ValidateSpec_Gate4_AddressOverflowGuard) {
    ModbusExecuteSpec spec;
    spec.slaveId = 1;
    spec.functionCode = 0x03;

    // Exactly touching 65536
    spec.startAddress = 65530;
    spec.quantity = 6; // 65530 + 6 = 65536
    EXPECT_TRUE(service_->validateSpec(spec).isValid);

    // Exceeding 65536 (overflow)
    spec.startAddress = 65530;
    spec.quantity = 7; // 65530 + 7 = 65537
    auto res = service_->validateSpec(spec);
    EXPECT_FALSE(res.isValid);
    EXPECT_TRUE(res.errorMessage.contains("overflow"));
}

TEST_F(RequestSubmissionServiceTest, ValidateSpec_Gate5_WirePayloadDefense) {
    ModbusExecuteSpec spec;
    spec.slaveId = 1;
    spec.startAddress = 0;

    // FC 05 coil value validation
    spec.functionCode = 0x05;
    spec.quantity = 1;
    spec.writeData = QByteArray("\x12\x34", 2); // Invalid coil value (must be 0x0000 or 0xFF00)
    EXPECT_FALSE(service_->validateSpec(spec).isValid);

    spec.writeData = QByteArray("\xFF\x00", 2); // Valid ON
    EXPECT_TRUE(service_->validateSpec(spec).isValid);

    spec.writeData = QByteArray("\x00\x00", 2); // Valid OFF
    EXPECT_TRUE(service_->validateSpec(spec).isValid);

    // FC 0F size match
    spec.functionCode = 0x0F;
    spec.quantity = 9; // requires ceil(9/8) = 2 bytes
    spec.writeData = QByteArray("\x0B", 1); // 1 byte provided
    EXPECT_FALSE(service_->validateSpec(spec).isValid);

    spec.writeData = QByteArray("\x0B\x01", 2); // 2 bytes provided
    EXPECT_TRUE(service_->validateSpec(spec).isValid);

    // FC 10 size match
    spec.functionCode = 0x10;
    spec.quantity = 2; // requires 2 * 2 = 4 bytes
    spec.writeData = QByteArray("\x01\x02\x03", 3);
    EXPECT_FALSE(service_->validateSpec(spec).isValid);

    spec.writeData = QByteArray("\x01\x02\x03\x04", 4);
    EXPECT_TRUE(service_->validateSpec(spec).isValid);
}

TEST_F(RequestSubmissionServiceTest, ValidateSpec_RawMode) {
    ModbusExecuteSpec spec;
    spec.isRawMode = true;

    // Empty raw data
    spec.rawPduOrAdu.clear();
    EXPECT_FALSE(service_->validateSpec(spec).isValid);

    // Valid raw data
    spec.rawPduOrAdu = QByteArray("\x01\x03\x00\x00\x00\x0A\xC5\xCD", 8);
    EXPECT_TRUE(service_->validateSpec(spec).isValid);

    // Oversized (> 260)
    spec.rawPduOrAdu = QByteArray(261, 'A');
    EXPECT_FALSE(service_->validateSpec(spec).isValid);
}

TEST_F(RequestSubmissionServiceTest, BuildExecuteRequest_EndToEnd) {
    // Read request execution
    ModbusExecuteSpec readSpec;
    readSpec.slaveId = 1;
    readSpec.functionCode = 0x03;
    readSpec.startAddress = 100;
    readSpec.quantity = 5;

    auto readRes = service_->buildExecuteRequest(readSpec);
    EXPECT_TRUE(readRes.ok);
    EXPECT_GT(readRes.requestId, 0);
    EXPECT_EQ(static_cast<uint8_t>(readRes.pdu.functionCode()), 0x03);

    auto info = service_->lookup(readRes.requestId);
    ASSERT_TRUE(info.has_value());
    EXPECT_EQ(info->address, 100);

    // Write request execution
    ModbusExecuteSpec writeSpec;
    writeSpec.slaveId = 1;
    writeSpec.functionCode = 0x06;
    writeSpec.startAddress = 200;
    writeSpec.quantity = 1;
    writeSpec.writeData = QByteArray("\x12\x34", 2);

    auto writeRes = service_->buildExecuteRequest(writeSpec);
    EXPECT_TRUE(writeRes.ok);
    EXPECT_EQ(static_cast<uint8_t>(writeRes.pdu.functionCode()), 0x06);

    // Raw mode execution
    ModbusExecuteSpec rawSpec;
    rawSpec.isRawMode = true;
    rawSpec.rawPduOrAdu = QByteArray("\x03\x00\x64\x00\x05", 5);

    auto rawRes = service_->buildExecuteRequest(rawSpec);
    EXPECT_TRUE(rawRes.ok);
    EXPECT_EQ(static_cast<uint8_t>(rawRes.pdu.functionCode()), 0x03);

    // Invalid spec fails cleanly
    ModbusExecuteSpec invalidSpec;
    invalidSpec.slaveId = 0;
    invalidSpec.functionCode = 0x03; // broadcast read rejected
    auto invRes = service_->buildExecuteRequest(invalidSpec);
    EXPECT_FALSE(invRes.ok);
    EXPECT_FALSE(invRes.error.isEmpty());
}

} // namespace
