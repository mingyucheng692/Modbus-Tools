// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "modbus/session/FrameExtractor.h"
#include "modbus/transport/ModbusSerialTransport.h"
#include "modbus/base/ModbusCrc.h"
#include "helpers/ModbusTestHelpers.h"

#include <chrono>

using namespace modbus::session;
using namespace modbus::base;
using namespace modbus::transport;

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

    const QByteArray frame(":010302007B7F\r\n");
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

    extractor.feed(QByteArray("noise:010302007B7F\r\n"));

    ASSERT_TRUE(extractor.hasCompleteFrame());
    const auto extracted = extractor.popFrame();
    ASSERT_TRUE(extracted.has_value());
    EXPECT_EQ(*extracted, QByteArray(":010302007B7F\r\n"));
}

TEST(FrameExtractorTest, FrameDuration_ZeroInputsYieldZero) {
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::RTU);
    cfg.baudRate = 0;
    EXPECT_EQ(FrameExtractor::calculateFrameDuration(cfg, 100).count(), 0);

    cfg.baudRate = 9600;
    EXPECT_EQ(FrameExtractor::calculateFrameDuration(cfg, 0).count(), 0);
}

TEST(FrameExtractorTest, FrameDuration_ScalesWithBaudAndFrameLength) {
    // 8N1: 1 start + 8 data + 0 parity + 1 stop = 10 bits per character.
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::RTU);
    cfg.baudRate = 9600;
    cfg.dataBits = 8;
    cfg.parity = 0;
    cfg.stopBits = 1;

    // 7 bytes * 10 bits / 9600 = 7291.67us -> ceil 7292us
    EXPECT_EQ(FrameExtractor::calculateFrameDuration(cfg, 7).count(), 7292);

    // 21 bytes * 10 bits / 300 = 700000us exactly (T07 slow-wire scenario).
    cfg.baudRate = 300;
    EXPECT_EQ(FrameExtractor::calculateFrameDuration(cfg, 21).count(), 700000);

    // 37 bytes * 10 bits / 300 = 1233333.33us -> ceil 1233334us
    EXPECT_EQ(FrameExtractor::calculateFrameDuration(cfg, 37).count(), 1233334);
}

// Documented RTU resync strategy (see FrameExtractor.h): frames are
// delimited by t3.5 silence alone. Two frames arriving without a gap merge
// into one buffer; the merged buffer fails its CRC and is dropped as a
// whole — no sliding-window resync. Recovery happens via the request retry.
TEST(FrameExtractorTest, RtuMergedFrames_SingleBufferRejectedWhole) {
    FrameExtractor extractor(ModbusMode::RTU, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::RTU);
    cfg.baudRate = 9600;
    extractor.setConfig(cfg);

    const auto frameA = makeRtuAdu(1, 0x03, QByteArray::fromHex("020001"));
    const auto frameB = makeRtuAdu(1, 0x03, QByteArray::fromHex("020002"));
    extractor.feed(frameA + frameB); // single arrival -> merged buffer

    const auto boundary = extractor.nextRtuFrameBoundary();
    ASSERT_NE(boundary, std::chrono::steady_clock::time_point{});

    const auto merged = extractor.tryPopRtuResponseFrame(boundary);
    ASSERT_TRUE(merged.has_value());
    EXPECT_EQ(*merged, frameA + frameB); // one buffer, not two frames

    // The merged buffer fails the CRC check at the transport boundary.
    ModbusSerialTransport transport(SerialFraming::Rtu);
    (void)transport.buildRequest(modbus::test::MakeReadHoldingRegistersPdu(0, 1), 1);
    const auto parsed = transport.parseResponse(*merged);
    EXPECT_EQ(parsed.status, ParseResponseStatus::Invalid);
}

TEST(FrameExtractorTest, TcpCoalescedFrames_BothExtractedInOrder) {
    FrameExtractor extractor(ModbusMode::TCP, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    extractor.setConfig(cfg);

    const auto frameA = makeTcpAdu(1, 1, 0x03, QByteArray::fromHex("020001"));
    const auto frameB = makeTcpAdu(2, 1, 0x03, QByteArray::fromHex("020002"));
    extractor.feed(frameA + frameB); // both frames in one arrival

    ASSERT_TRUE(extractor.hasCompleteFrame());
    const auto first = extractor.popFrame();
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(*first, frameA);
    const auto second = extractor.popFrame();
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(*second, frameB);
    EXPECT_FALSE(extractor.hasCompleteFrame());
    EXPECT_EQ(extractor.bufferSize(), 0);
}

TEST(FrameExtractorTest, TcpFragmentedFrame_AssemblesAcrossFeeds) {
    FrameExtractor extractor(ModbusMode::TCP, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    extractor.setConfig(cfg);

    const auto adu = makeTcpAdu(7, 1, 0x03, QByteArray::fromHex("020001"));
    extractor.feed(adu.left(4)); // MBAP header only
    EXPECT_FALSE(extractor.hasCompleteFrame());
    extractor.feed(adu.mid(4, 3)); // partial payload
    EXPECT_FALSE(extractor.hasCompleteFrame());
    extractor.feed(adu.mid(7)); // remainder completes the frame

    ASSERT_TRUE(extractor.hasCompleteFrame());
    const auto frame = extractor.popFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, adu);
}

TEST(FrameExtractorTest, AsciiBadLrc_SkippedUpToNextFrame) {
    FrameExtractor extractor(ModbusMode::ASCII, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::ASCII);
    extractor.setConfig(cfg);

    const QByteArray badLrc = QByteArray(":010302007B00\r\n"); // LRC mismatch
    const QByteArray good = QByteArray(":010302007B7F\r\n");
    extractor.feed(badLrc + good);

    // The bad frame is not delivered; scanning resumes at the next ':' and
    // the following valid frame is extracted intact.
    ASSERT_TRUE(extractor.hasCompleteFrame());
    const auto frame = extractor.popFrame();
    ASSERT_TRUE(frame.has_value());
    EXPECT_EQ(*frame, good);
    EXPECT_FALSE(extractor.hasCompleteFrame());
    EXPECT_GT(extractor.droppedInvalidBytes(), 0);
}

TEST(FrameExtractorTest, RtuOversizeOverflow_DiscardsAndCounts) {
    FrameExtractor extractor(ModbusMode::RTU, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::RTU);
    cfg.baudRate = 9600;
    extractor.setConfig(cfg);

    // One arrival larger than the RTU ADU cap: the whole buffer is dropped
    // and its byte count must land in the drop accounting.
    QByteArray oversized(300, '\x55');
    ASSERT_GT(static_cast<int>(oversized.size()), 260);
    extractor.feed(oversized);

    EXPECT_EQ(extractor.bufferSize(), 0);
    EXPECT_FALSE(extractor.hasCompleteFrame());
    EXPECT_EQ(extractor.droppedInvalidBytes(), 300);
}

TEST(FrameExtractorTest, TcpDropLimit_TripsAndReports) {
    FrameExtractor extractor(ModbusMode::TCP, 9600);
    auto cfg = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    extractor.setConfig(cfg);

    // Garbage that never forms a valid MBAP header: each byte is dropped and
    // counted until the limit trips, then the buffer is flushed.
    QByteArray garbage(300, '\x00');
    garbage[0] = '\x7F'; // length field far beyond any legal frame
    extractor.feed(garbage);

    EXPECT_TRUE(extractor.hasExceededDropLimit());
    EXPECT_EQ(extractor.bufferSize(), 0);
    EXPECT_GT(extractor.droppedInvalidBytes(), 256);
}
