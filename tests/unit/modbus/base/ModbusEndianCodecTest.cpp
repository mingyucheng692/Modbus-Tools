#include <gtest/gtest.h>

#include "modbus/base/ModbusEndianCodec.h"
#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusLrc.h"
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
    EXPECT_EQ(result.value().toByteArray().toHex().toUpper(), QByteArray("0312340002"));
}

TEST(ModbusEndianCodecTest, BuildWriteSingleRegisterEncodesBigEndianPayload)
{
    const auto result = ModbusPduBuilder::buildWriteSingleRegister(0x0010, 0xABCD);

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.value().toByteArray().toHex().toUpper(), QByteArray("060010ABCD"));
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

// ============================================================================
// inspectTcpAdu — branch coverage: incomplete, invalid, boundary
// ============================================================================

TEST(ModbusEndianCodecTest, InspectTcpAdu_IncompleteHeader_ReturnsZero) {
    const QByteArray adu = QByteArray::fromHex("1234"); // only 2 bytes (< 6)
    EXPECT_EQ(inspectTcpAdu(QByteArrayView(adu)), 0);
}

TEST(ModbusEndianCodecTest, InspectTcpAdu_ProtocolIdNonZero_ReturnsMinusOne) {
    const QByteArray adu = QByteArray::fromHex("1234000100061103006B0003");
    EXPECT_EQ(inspectTcpAdu(QByteArrayView(adu)), -1);
}

TEST(ModbusEndianCodecTest, InspectTcpAdu_LengthTooSmall_ReturnsMinusOne) {
    const QByteArray adu = QByteArray::fromHex("1234000000011103006B0003");
    EXPECT_EQ(inspectTcpAdu(QByteArrayView(adu)), -1);
}

TEST(ModbusEndianCodecTest, InspectTcpAdu_LengthExceedsMax_ReturnsMinusOne) {
    // length=0xFFFF > kMaxTcpMbapLength (254)
    const QByteArray adu = QByteArray::fromHex("12340000FFFF1103006B0003");
    EXPECT_EQ(inspectTcpAdu(QByteArrayView(adu)), -1);
}

TEST(ModbusEndianCodecTest, InspectTcpAdu_IncompleteFrame_ReturnsZero) {
    // length field says 6, but only 8 bytes provided (need 12)
    const QByteArray adu = QByteArray::fromHex("12340000000611");
    EXPECT_EQ(inspectTcpAdu(QByteArrayView(adu)), 0);
}

TEST(ModbusEndianCodecTest, InspectTcpAdu_NullFields_ReturnsFullLength) {
    // fields=nullptr should still return valid fullLength
    const QByteArray adu = QByteArray::fromHex("1234000000061103006B0003");
    EXPECT_EQ(inspectTcpAdu(QByteArrayView(adu), nullptr), adu.size());
}

TEST(ModbusEndianCodecTest, InspectTcpAdu_LengthAtMinBoundary_Accepted) {
    // length=2 (minimum valid), frame: 6 + 2 = 8 bytes
    const QByteArray adu = QByteArray::fromHex("1234000000020101");
    EXPECT_EQ(inspectTcpAdu(QByteArrayView(adu)), adu.size());
}

TEST(ModbusEndianCodecTest, InspectTcpAdu_LengthAtMaxBoundary_Accepted) {
    // length=254 (kMaxTcpMbapLength), frame: 6 + 254 = 260 bytes
    QByteArray adu(6 + 254, '\x00');
    reinterpret_cast<char*>(adu.data())[0] = '\x12';
    reinterpret_cast<char*>(adu.data())[1] = '\x34'; // TID
    reinterpret_cast<char*>(adu.data())[4] = '\x00';
    reinterpret_cast<char*>(adu.data())[5] = '\xFE'; // length=254
    EXPECT_EQ(inspectTcpAdu(QByteArrayView(adu)), adu.size());
}

// ============================================================================
// inspectRtuAdu — branch coverage: incomplete, CRC mismatch, null fields
// ============================================================================

TEST(ModbusEndianCodecTest, InspectRtuAdu_IncompleteHeader_ReturnsZero) {
    const QByteArray adu = QByteArray::fromHex("0103"); // only 2 bytes (< 5)
    EXPECT_EQ(inspectRtuAdu(QByteArrayView(adu)), 0);
}

TEST(ModbusEndianCodecTest, InspectRtuAdu_CrcMismatch_ReturnsMinusOne) {
    // Valid frame: 010304007B01C88A2C, CRC=0x2C8A (little-endian: 8A 2C)
    // Corrupt the CRC to force mismatch
    const QByteArray adu = QByteArray::fromHex("010304007B01C80000"); // CRC=0x0000
    EXPECT_EQ(inspectRtuAdu(QByteArrayView(adu)), -1);
}

TEST(ModbusEndianCodecTest, InspectRtuAdu_NullFields_ReturnsSize) {
    const QByteArray adu = QByteArray::fromHex("010304007B01C88A2C");
    EXPECT_EQ(inspectRtuAdu(QByteArrayView(adu), nullptr), adu.size());
}

TEST(ModbusEndianCodecTest, CalculateAsciiLrc_ComputesExpectedChecksum) {
    const QByteArray aduWithoutLrc = QByteArray::fromHex("010300000001");
    EXPECT_EQ(calculateModbusAsciiLrc(aduWithoutLrc), 0xFB);
}

TEST(ModbusEndianCodecTest, InspectAsciiAdu_ReadsDecodedFields) {
    AsciiAduFields fields;
    const QByteArray frame(":010302007BFD\r\n");

    const int fullLength = inspectAsciiAdu(QByteArrayView(frame), &fields);

    EXPECT_EQ(fullLength, frame.size());
    EXPECT_EQ(fields.slaveId, 0x01);
    EXPECT_EQ(fields.functionCode, 0x03);
    EXPECT_EQ(fields.receivedLrc, 0xFD);
    EXPECT_EQ(fields.calculatedLrc, 0xFD);
    EXPECT_EQ(fields.binaryAdu, QByteArray::fromHex("010302007BFD"));
}

TEST(ModbusEndianCodecTest, InspectAsciiAdu_InvalidLrc_ReturnsMinusOne) {
    const QByteArray frame(":010302007BFC\r\n");
    EXPECT_EQ(inspectAsciiAdu(QByteArrayView(frame)), -1);
}

// ============================================================================
// validateResponsePdu — exception responses
// ============================================================================

TEST(ModbusEndianCodecTest, ValidateResponsePdu_ExceptionFunctionCodeMismatch_ReturnsError) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    // Exception with wrong original FC (WriteSingleRegister instead of ReadHoldingRegisters)
    const Pdu response(FunctionCode::ReadCoils, ExceptionCode::IllegalFunction);
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_ExceptionPayloadTooLarge_ReturnsError) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    // isException() checks (FC & 0x80). Create Pdu with FC=0x83 (ReadHolding|0x80)
    // and 2 bytes of data (should be exactly 1 for exception)
    const Pdu response(static_cast<FunctionCode>(0x83), QByteArray::fromHex("0000"));
    EXPECT_TRUE(response.isException());
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_MatchingException_ReturnsEmpty) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    const Pdu response(FunctionCode::ReadHoldingRegisters, ExceptionCode::IllegalFunction);
    const QString error = validateResponsePdu(request, response);
    EXPECT_TRUE(error.isEmpty()) << error.toStdString();
}

// ============================================================================
// validateResponsePdu — function code mismatch (non-exception)
// ============================================================================

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WrongFunctionCode_ReturnsError) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    const Pdu response(FunctionCode::ReadCoils, QByteArray::fromHex("0100"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

// ============================================================================
// validateResponsePdu — ReadCoils / ReadDiscreteInputs branches
// ============================================================================

TEST(ModbusEndianCodecTest, ValidateResponsePdu_BitRead_InvalidRequestData_ReturnsError) {
    // request data too short for quantity read
    const Pdu request(FunctionCode::ReadCoils, QByteArray::fromHex("0000"));
    const Pdu response(FunctionCode::ReadCoils, QByteArray::fromHex("0110"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_BitRead_ResponseTooShort_ReturnsError) {
    const Pdu request(FunctionCode::ReadCoils, QByteArray::fromHex("00000008"));
    const Pdu response(FunctionCode::ReadCoils, QByteArray());
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_BitRead_ByteCountPayloadMismatch_ReturnsError) {
    // byteCount says 2 but only 1 byte follows (total 2 bytes, needs 3)
    const Pdu request(FunctionCode::ReadCoils, QByteArray::fromHex("00000008"));
    const Pdu response(FunctionCode::ReadCoils, QByteArray::fromHex("02AA"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_BitRead_ByteCountQuantityMismatch_ReturnsError) {
    // request quantity=8 means expected byteCount=1, but response byteCount=2
    const Pdu request(FunctionCode::ReadCoils, QByteArray::fromHex("00000008"));
    const Pdu response(FunctionCode::ReadCoils, QByteArray::fromHex("02AAAA"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_BitRead_Valid_ReturnsEmpty) {
    // request quantity=8 → expected byteCount=1
    const Pdu request(FunctionCode::ReadCoils, QByteArray::fromHex("00000008"));
    const Pdu response(FunctionCode::ReadCoils, QByteArray::fromHex("01A5"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_TRUE(error.isEmpty()) << error.toStdString();
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_BitRead_DiscreteInputs_ReturnsEmpty) {
    const Pdu request(FunctionCode::ReadDiscreteInputs, QByteArray::fromHex("00000010"));
    const Pdu response(FunctionCode::ReadDiscreteInputs, QByteArray::fromHex("020034"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_TRUE(error.isEmpty()) << error.toStdString();
}

// ============================================================================
// validateResponsePdu — ReadHoldingRegisters / ReadInputRegisters branches
// ============================================================================

TEST(ModbusEndianCodecTest, ValidateResponsePdu_RegisterRead_InvalidRequestData_ReturnsError) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("0000"));
    const Pdu response(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("0200000000"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_RegisterRead_ResponseTooShort_ReturnsError) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000002"));
    const Pdu response(FunctionCode::ReadHoldingRegisters, QByteArray());
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_RegisterRead_ByteCountPayloadMismatch_ReturnsError) {
    // byteCount=4 but only 2 bytes follow
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000002"));
    const Pdu response(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("041122"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_RegisterRead_ByteCountQuantityMismatch_ReturnsError) {
    // request qty=2 → expected 4 bytes, but byteCount=2
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000002"));
    const Pdu response(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("021122"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_RegisterRead_ValidSingle_ReturnsEmpty) {
    const Pdu request(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("00000001"));
    const Pdu response(FunctionCode::ReadHoldingRegisters, QByteArray::fromHex("020000"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_TRUE(error.isEmpty()) << error.toStdString();
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_RegisterRead_ReadInputRegisters_ReturnsEmpty) {
    const Pdu request(FunctionCode::ReadInputRegisters, QByteArray::fromHex("00000002"));
    const Pdu response(FunctionCode::ReadInputRegisters, QByteArray::fromHex("0400000000"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_TRUE(error.isEmpty()) << error.toStdString();
}

// ============================================================================
// validateResponsePdu — WriteSingleCoil / WriteSingleRegister branches
// ============================================================================

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WriteSingle_SizeMismatch_ReturnsError) {
    const Pdu request(FunctionCode::WriteSingleRegister, QByteArray::fromHex("00101234"));
    const Pdu response(FunctionCode::WriteSingleRegister, QByteArray::fromHex("0010"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WriteSingle_EchoMismatch_ReturnsError) {
    const Pdu request(FunctionCode::WriteSingleCoil, QByteArray::fromHex("0010FF00"));
    const Pdu response(FunctionCode::WriteSingleCoil, QByteArray::fromHex("00100000"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WriteSingleRegister_Valid_ReturnsEmpty) {
    const Pdu request(FunctionCode::WriteSingleRegister, QByteArray::fromHex("00101234"));
    const Pdu response(FunctionCode::WriteSingleRegister, QByteArray::fromHex("00101234"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_TRUE(error.isEmpty()) << error.toStdString();
}

// ============================================================================
// validateResponsePdu — WriteMultipleCoils / WriteMultipleRegisters branches
// ============================================================================

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WriteMultiple_InvalidRequestData_ReturnsError) {
    const Pdu request(FunctionCode::WriteMultipleRegisters, QByteArray::fromHex("0010"));
    const Pdu response(FunctionCode::WriteMultipleRegisters, QByteArray::fromHex("00100002"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WriteMultiple_ResponseSizeMismatch_ReturnsError) {
    const Pdu request(FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("001000020400010002"));
    const Pdu response(FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("001000020000"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WriteMultiple_IncompleteResponseFields_ReturnsError) {
    const Pdu request(FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("001000020400010002"));
    const Pdu response(FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("0010"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WriteMultiple_AddressMismatch_ReturnsError) {
    const Pdu request(FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("001000020400010002"));
    const Pdu response(FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("00200002"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WriteMultiple_QuantityMismatch_ReturnsError) {
    const Pdu request(FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("001000020400010002"));
    const Pdu response(FunctionCode::WriteMultipleRegisters,
        QByteArray::fromHex("00100001"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

TEST(ModbusEndianCodecTest, ValidateResponsePdu_WriteMultipleCoils_Valid_ReturnsEmpty) {
    const Pdu request(FunctionCode::WriteMultipleCoils,
        QByteArray::fromHex("001000080100"));
    const Pdu response(FunctionCode::WriteMultipleCoils,
        QByteArray::fromHex("00100008"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_TRUE(error.isEmpty()) << error.toStdString();
}

// ============================================================================
// validateResponsePdu — unsupported function code (default branch)
// ============================================================================

TEST(ModbusEndianCodecTest, ValidateResponsePdu_UnsupportedFunctionCode_ReturnsError) {
    // Use a function code that's not handled by any switch case
    // ReadFileRecord (0x14/20) is not in the switch
    const Pdu request(static_cast<FunctionCode>(0x14), QByteArray::fromHex("0000"));
    const Pdu response(static_cast<FunctionCode>(0x14), QByteArray::fromHex("0000"));
    const QString error = validateResponsePdu(request, response);
    EXPECT_FALSE(error.isEmpty());
}

// ============================================================================
// readBigEndian<T> template — the consolidated big-endian reader.
// Covers uint16/uint32 normal reads, boundary fits, out-of-range offsets,
// empty data, and the "value unchanged on failure" contract.
// ============================================================================

TEST(ModbusEndianCodecTest, ReadBigEndian_Uint16_ReadsValueAtOffset) {
    const QByteArray data = QByteArray::fromHex("12345678");
    uint16_t value = 0;
    EXPECT_TRUE(readBigEndian<uint16_t>(QByteArrayView(data), 0, value));
    EXPECT_EQ(value, 0x1234u);
    EXPECT_TRUE(readBigEndian<uint16_t>(QByteArrayView(data), 2, value));
    EXPECT_EQ(value, 0x5678u);
}

TEST(ModbusEndianCodecTest, ReadBigEndian_Uint32_ReadsValueAtOffset) {
    const QByteArray data = QByteArray::fromHex("1234567890ABCDEF");
    uint32_t value = 0;
    EXPECT_TRUE(readBigEndian<uint32_t>(QByteArrayView(data), 0, value));
    EXPECT_EQ(value, 0x12345678u);
    EXPECT_TRUE(readBigEndian<uint32_t>(QByteArrayView(data), 4, value));
    EXPECT_EQ(value, 0x90ABCDEFu);
}

TEST(ModbusEndianCodecTest, ReadBigEndian_FitsExactlyAtEndOfBuffer) {
    // 2-byte buffer, uint16 read at offset 0 exactly fits (offset + sizeof == size).
    const QByteArray data = QByteArray::fromHex("ABCD");
    uint16_t value = 0;
    EXPECT_TRUE(readBigEndian<uint16_t>(QByteArrayView(data), 0, value));
    EXPECT_EQ(value, 0xABCDu);
}

TEST(ModbusEndianCodecTest, ReadBigEndian_OffsetZero_NegativeOffset_Fails) {
    const QByteArray data = QByteArray::fromHex("ABCD");
    uint16_t value = 99;
    EXPECT_FALSE(readBigEndian<uint16_t>(QByteArrayView(data), -1, value));
    // Contract: value is left unchanged when bounds check fails.
    EXPECT_EQ(value, 99u);
}

TEST(ModbusEndianCodecTest, ReadBigEndian_OffsetBeyondSize_Fails) {
    const QByteArray data = QByteArray::fromHex("ABCD"); // size 2
    uint16_t value = 99;
    // offset 2 + sizeof(uint16)=2 == 4 > size 2 → fail
    EXPECT_FALSE(readBigEndian<uint16_t>(QByteArrayView(data), 2, value));
    EXPECT_EQ(value, 99u);
    // offset 1 + sizeof(uint16)=2 == 3 > size 2 → fail
    EXPECT_FALSE(readBigEndian<uint16_t>(QByteArrayView(data), 1, value));
    EXPECT_EQ(value, 99u);
}

TEST(ModbusEndianCodecTest, ReadBigEndian_EmptyData_Fails) {
    const QByteArray data;
    uint16_t value = 99;
    EXPECT_FALSE(readBigEndian<uint16_t>(QByteArrayView(data), 0, value));
    EXPECT_EQ(value, 99u);
}

TEST(ModbusEndianCodecTest, ReadBigEndian_Uint32_TruncatedBuffer_Fails) {
    // 3-byte buffer cannot hold a uint32 at offset 0.
    const QByteArray data = QByteArray::fromHex("123456");
    uint32_t value = 99;
    EXPECT_FALSE(readBigEndian<uint32_t>(QByteArrayView(data), 0, value));
    EXPECT_EQ(value, 99u);
}
