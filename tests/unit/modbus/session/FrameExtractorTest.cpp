#include <gtest/gtest.h>

#include "modbus/session/FrameExtractor.h"
#include "modbus/base/ModbusCrc.h"
#include "helpers/ModbusTestHelpers.h"

#include <chrono>

using namespace modbus::session;
using namespace modbus::base;

namespace {

QByteArray makeTcpAdu(uint16_t transactionId,
                      uint8_t unitId,
                      uint8_t functionCode,
                      const QByteArray& payload) {
    QByteArray adu;
    const uint16_t length = static_cast<uint16_t>(1 + 1 + payload.size());
    adu.append(static_cast<char>((transactionId >> 8) & 0xFF));
    adu.append(static_cast<char>(transactionId & 0xFF));
    adu.append('\x00');
    adu.append('\x00');
    adu.append(static_cast<char>((length >> 8) & 0xFF));
    adu.append(static_cast<char>(length & 0xFF));
    adu.append(static_cast<char>(unitId));
    adu.append(static_cast<char>(functionCode));
    adu.append(payload);
    return adu;
}

QByteArray makeRtuAdu(uint8_t slaveId,
                      uint8_t functionCode,
                      const QByteArray& payload) {
    QByteArray adu;
    adu.append(static_cast<char>(slaveId));
    adu.append(static_cast<char>(functionCode));
    adu.append(payload);

    const uint16_t crc = calculateModbusRtuCrc(adu);
    adu.append(static_cast<char>(crc & 0xFF));
    adu.append(static_cast<char>((crc >> 8) & 0xFF));
    return adu;
}

} // namespace

TEST(FrameExtractorTest, TcpFeedCompleteFrame_ExtractsImmediately) {
    FrameExtractor extractor(ModbusMode::TCP, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    extractor.setConfig(cfg);

    const QByteArray adu = makeTcpAdu(/*transactionId=*/7,
                                      /*unitId=*/1,
                                      /*functionCode=*/0x03,
                                      QByteArray::fromHex("020001"));

    extractor.feed(adu);

    ASSERT_TRUE(extractor.hasCompleteFrame());
    const auto frame = extractor.popFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, adu);
    EXPECT_EQ(extractor.bufferSize(), 0);
}

TEST(FrameExtractorTest, TcpInvalidMbap_IncrementsDroppedCount) {
    FrameExtractor extractor(ModbusMode::TCP, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    extractor.setConfig(cfg);

    // Invalid TCP frame: protocol id != 0.
    extractor.feed(QByteArray::fromHex("000100010006010300000001"));

    EXPECT_FALSE(extractor.hasCompleteFrame());
    EXPECT_GT(extractor.droppedInvalidBytes(), 0);
}

TEST(FrameExtractorTest, RtuResponseFrame_BecomesAvailableAfterInterFrameDelay) {
    FrameExtractor extractor(ModbusMode::RTU, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::RTU);
    cfg.baudRate = 9600;
    extractor.setConfig(cfg);

    const QByteArray adu = makeRtuAdu(/*slaveId=*/1,
                                      /*functionCode=*/0x03,
                                      QByteArray::fromHex("020001"));
    extractor.feed(adu);

    const auto boundary = extractor.nextRtuFrameBoundary();
    ASSERT_NE(boundary, std::chrono::steady_clock::time_point{});

    EXPECT_FALSE(extractor.tryPopRtuResponseFrame(boundary - std::chrono::microseconds(1))
                     .has_value());

    const auto frame = extractor.tryPopRtuResponseFrame(boundary);
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, adu);
    EXPECT_EQ(extractor.bufferSize(), 0);
}

TEST(FrameExtractorTest, InterFrameDelayOverride_TakesPrecedence) {
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::RTU);
    cfg.baudRate = 9600;
    cfg.interFrameDelayUs = 4321;

    EXPECT_EQ(FrameExtractor::calculateInterFrameDelay(cfg),
              std::chrono::microseconds(4321));
}

TEST(FrameExtractorTest, Reset_ClearsBufferedStateAndCounters) {
    FrameExtractor extractor(ModbusMode::TCP, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    extractor.setConfig(cfg);

    extractor.feed(QByteArray::fromHex("000100010006010300000001"));
    ASSERT_GT(extractor.droppedInvalidBytes(), 0);

    extractor.reset();

    EXPECT_FALSE(extractor.hasCompleteFrame());
    EXPECT_EQ(extractor.bufferSize(), 0);
    EXPECT_EQ(extractor.droppedInvalidBytes(), 0);
}

TEST(FrameExtractorTest, AsciiFeedCompleteFrame_ExtractsImmediately) {
    FrameExtractor extractor(ModbusMode::ASCII, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::ASCII);
    extractor.setConfig(cfg);

    const QByteArray frame(":010302007BFD\r\n");
    extractor.feed(frame);

    ASSERT_TRUE(extractor.hasCompleteFrame());
    const auto extracted = extractor.popFrame();
    ASSERT_TRUE(extracted.has_value());
    EXPECT_EQ(*extracted, frame);
}

TEST(FrameExtractorTest, AsciiFeedWithLeadingNoise_RecoversNextFrame) {
    FrameExtractor extractor(ModbusMode::ASCII, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::ASCII);
    extractor.setConfig(cfg);

    extractor.feed(QByteArray("noise:010302007BFD\r\n"));

    ASSERT_TRUE(extractor.hasCompleteFrame());
    const auto extracted = extractor.popFrame();
    ASSERT_TRUE(extracted.has_value());
    EXPECT_EQ(*extracted, QByteArray(":010302007BFD\r\n"));
}
