// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include "helpers/ModbusTestHelpers.h"
#include "modbus/base/ModbusCrc.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "modbus/session/FrameExtractor.h"
#include "modbus/transport/ModbusSerialTransport.h"
#include "modbus/transport/ModbusTcpTransport.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <vector>

using namespace modbus::base;
using namespace modbus::parser;
using namespace modbus::session;
using namespace modbus::transport;

namespace {

constexpr int kFuzzIterations = 10000;

QByteArray makeRandomBytes(std::mt19937& rng, int maxSize)
{
    std::uniform_int_distribution<int> sizeDist(0, maxSize);
    std::uniform_int_distribution<int> byteDist(0, 255);

    const int size = sizeDist(rng);
    QByteArray data(size, Qt::Uninitialized);
    for (int i = 0; i < size; ++i) {
        data[i] = static_cast<char>(byteDist(rng));
    }
    return data;
}

QByteArray makeValidTcpResponse()
{
    QByteArray adu = QByteArray::fromHex("000100000005010302007B");
    return adu;
}

QByteArray makeValidRtuResponse()
{
    QByteArray adu = QByteArray::fromHex("010302007B");
    const uint16_t crc = calculateModbusRtuCrc(adu);
    adu.append(static_cast<char>(crc & 0xFF));
    adu.append(static_cast<char>((crc >> 8) & 0xFF));
    return adu;
}

std::vector<QByteArray> buildSeedCorpus()
{
    std::vector<QByteArray> corpus;
    corpus.push_back(QByteArray());
    corpus.push_back(QByteArray::fromHex("00"));
    corpus.push_back(QByteArray::fromHex("010300"));
    corpus.push_back(QByteArray::fromHex("000100010006010300000001"));
    corpus.push_back(makeValidTcpResponse());
    corpus.push_back(makeValidRtuResponse());
    return corpus;
}

} // namespace

TEST(ModbusParsingFuzzTest, ParserAndIntegritySurfaces_HandleDeterministicCorpus)
{
    ModbusTcpTransport tcpTransport;
    ModbusSerialTransport rtuTransport(SerialFraming::Rtu);
    std::mt19937 rng(0xC0FFEE);

    std::vector<QByteArray> corpus = buildSeedCorpus();
    for (int i = 0; i < kFuzzIterations; ++i) {
        corpus.push_back(makeRandomBytes(rng, 300));
    }

    // Seed sanity: the valid corpus entries must actually parse — guards the
    // fuzzer itself against silently rotting into a garbage-only corpus.
    EXPECT_TRUE(parse(makeValidTcpResponse(), ProtocolType::Tcp).isValid);
    EXPECT_TRUE(parse(makeValidRtuResponse(), ProtocolType::Unknown).isValid);
    EXPECT_EQ(tcpTransport.checkIntegrity(makeValidTcpResponse()),
              static_cast<int>(makeValidTcpResponse().size()));
    EXPECT_GT(rtuTransport.checkIntegrity(makeValidRtuResponse()), 0);

    for (const QByteArray& frame : corpus) {
        // Behavioral contract: the parser is deterministic and total — the
        // same input always yields the same verdict, and a rejection carries
        // a diagnosis instead of failing silently.
        const auto first = parse(frame, ProtocolType::Unknown);
        const auto second = parse(frame, ProtocolType::Unknown);
        EXPECT_EQ(first.isValid, second.isValid);
        EXPECT_EQ(first.protocol, second.protocol);
        EXPECT_EQ(first.type, second.type);
        if (!first.isValid) {
            EXPECT_FALSE(first.error.isEmpty());
        }

        // Protocol-forced parses agree with auto-detection when they accept.
        const auto asTcp = parse(frame, ProtocolType::Tcp);
        if (asTcp.isValid) {
            EXPECT_EQ(asTcp.protocol, ProtocolType::Tcp);
        }
        const auto asRtu = parse(frame, ProtocolType::Rtu, 0, 0, true);
        if (asRtu.isValid) {
            EXPECT_EQ(asRtu.protocol, ProtocolType::Rtu);
        }

        (void)tcpTransport.checkIntegrity(frame);
        (void)rtuTransport.checkIntegrity(frame);
    }
}

TEST(ModbusParsingFuzzTest, FrameExtractorHandlesDeterministicRandomChunks)
{
    std::mt19937 rng(0x5EED1234);

    FrameExtractor tcpExtractor(ModbusMode::TCP, 9600);
    auto tcpConfig = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    tcpExtractor.setConfig(tcpConfig);

    FrameExtractor rtuExtractor(ModbusMode::RTU, 9600);
    auto rtuConfig = modbus::test::MakeModbusConfig(ModbusMode::RTU);
    rtuConfig.baudRate = 9600;
    rtuExtractor.setConfig(rtuConfig);

    std::vector<QByteArray> corpus = buildSeedCorpus();
    for (int i = 0; i < kFuzzIterations; ++i) {
        corpus.push_back(makeRandomBytes(rng, 96));
    }

    for (const QByteArray& chunk : corpus) {
        EXPECT_NO_THROW({
            tcpExtractor.feed(chunk);
            while (tcpExtractor.hasCompleteFrame()) {
                (void)tcpExtractor.popFrame();
            }

            rtuExtractor.feed(chunk);
            const auto boundary = rtuExtractor.nextRtuFrameBoundary();
            if (boundary != std::chrono::steady_clock::time_point{}) {
                (void)rtuExtractor.tryPopRtuResponseFrame(boundary + std::chrono::microseconds(1));
            }

            if (chunk.size() > 80) {
                tcpExtractor.reset();
                rtuExtractor.reset();
            }
        });
    }

    // State-reset contract: after reset() the extractor is fully clean —
    // no buffered bytes, no pending frames, no residual drop accounting.
    tcpExtractor.reset();
    rtuExtractor.reset();
    EXPECT_EQ(tcpExtractor.bufferSize(), 0);
    EXPECT_FALSE(tcpExtractor.hasCompleteFrame());
    EXPECT_EQ(tcpExtractor.droppedInvalidBytes(), 0);
    EXPECT_EQ(rtuExtractor.bufferSize(), 0);
    EXPECT_FALSE(rtuExtractor.hasCompleteFrame());
    EXPECT_EQ(rtuExtractor.droppedInvalidBytes(), 0);
    EXPECT_EQ(rtuExtractor.nextRtuFrameBoundary(), std::chrono::steady_clock::time_point{});
}
