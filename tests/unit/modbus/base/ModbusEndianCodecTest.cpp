#include <gtest/gtest.h>

#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusPduBuilder.h"
#include "modbus/base/ModbusProtocolChecks.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "modbus/session/RequestValidator.h"

using namespace modbus::base;

TEST(ModbusEndianCodecTest, InspectTcpAduReadsBigEndianHeaderFields)
{
    TcpAduFields fields;
    const QByteArray adu = QByteArray::fromHex("1234000000061103006B0003");

    const int fullLength = inspectTcpAdu(QByteArrayView(adu), &fields);

    EXPECT_EQ(fullLength, adu.size());
    EXPECT_EQ(fields.transactionId, 0x1234);
    EXPECT_EQ(fields.protocolId, 0x0000);
    EXPECT_EQ(fields.length, 0x0006);
    EXPECT_EQ(fields.unitId, 0x11);
}

TEST(ModbusEndianCodecTest, InspectRtuAduReadsLittleEndianCrc)
{
    RtuAduFields fields;
    const QByteArray adu = QByteArray::fromHex("010304007B01C88A2C");

    const int fullLength = inspectRtuAdu(QByteArrayView(adu), &fields);

    EXPECT_EQ(fullLength, adu.size());
    EXPECT_EQ(fields.slaveId, 0x01);
    EXPECT_EQ(fields.functionCode, 0x03);
    EXPECT_EQ(fields.receivedCrc, 0x2C8A);
    EXPECT_EQ(fields.calculatedCrc, 0x2C8A);
}

TEST(ModbusEndianCodecTest, BuildReadRequestEncodesBigEndianPayload)
{
    const auto result = ModbusPduBuilder::buildReadRequest(
        FunctionCode::ReadHoldingRegisters, 0x1234, 0x0002);

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.pdu->toByteArray().toHex().toUpper(), QByteArray("0312340002"));
}

TEST(ModbusEndianCodecTest, BuildWriteSingleRegisterEncodesBigEndianPayload)
{
    const auto result = ModbusPduBuilder::buildWriteSingleRegister(0x0010, 0xABCD);

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.pdu->toByteArray().toHex().toUpper(), QByteArray("060010ABCD"));
}

TEST(ModbusEndianCodecTest, ValidateResponsePduAcceptsMatchingWriteMultipleEcho)
{
    const Pdu request(
        FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("001000020400010002"));
    const Pdu response(
        FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("00100002"));

    const QString error = validateResponsePdu(request, response);

    EXPECT_TRUE(error.isEmpty()) << error.toStdString();
}

TEST(ModbusEndianCodecTest, FrameParserExtractsStartAddressFromBigEndianRequestPdu)
{
    modbus::core::parser::ParseResult result;
    result.isValid = true;

    modbus::core::parser::ModbusFrameParser::parsePdu(
        result,
        QByteArray::fromHex("0300100002"),
        0,
        0);

    ASSERT_TRUE(result.isValid) << result.error.toStdString();
    ASSERT_EQ(result.dataItems.size(), 1);
    EXPECT_EQ(result.dataItems[0].address, 0x0010);
    EXPECT_EQ(result.expectedQuantity, 0x0002);
}

TEST(ModbusEndianCodecTest, RequestValidatorAcceptsValidReadRequestPayload)
{
    modbus::session::RequestValidator validator;
    const Pdu request(
        FunctionCode::ReadHoldingRegisters,
        QByteArray::fromHex("00100002"));

    const auto result = validator.validate(request, 1, ModbusMode::TCP);

    EXPECT_TRUE(result.valid) << result.error.toStdString();
    EXPECT_TRUE(result.error.isEmpty());
}
