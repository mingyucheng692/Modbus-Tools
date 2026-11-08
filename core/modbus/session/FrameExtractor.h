// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "../base/ModbusConfig.h"
#include "../base/ModbusTypes.h"
#include <QByteArray>
#include <deque>
#include <chrono>
#include <optional>

namespace modbus::session {

// Role: Byte-stream frame boundary detection for RTU / ASCII / TCP.
// Thread-safety: Owned and driven exclusively by RequestExecutor on the worker thread.
//
// RTU resync strategy (design decision): RTU frames are delimited solely by
// the t3.5 inter-frame silence. If event-loop latency ever exceeds t3.5, two
// logical frames merge into one buffer; the merged buffer fails its CRC and
// is dropped as a whole — no sliding-window resync is attempted, because a
// CRC-failure rescan is ambiguous (CRC16 is not a frame delimiter) and
// quadratic in the worst case. Recovery is delegated to the request layer's
// bounded retry; invalid data is counted and observable via
// droppedInvalidBytes() so the failure mode stays visible in the field.
class FrameExtractor {
public:
    explicit FrameExtractor(modbus::base::ModbusMode mode, int baudRate);

    void feed(QByteArrayView data);

    [[nodiscard]] bool hasCompleteFrame() const;
    std::optional<QByteArray> popFrame();
    void reset();
    void setConfig(const base::ModbusConfig& config);
    [[nodiscard]] qsizetype bufferSize() const;

    [[nodiscard]] bool isRtuFrameReadyToParse(
        std::chrono::steady_clock::time_point now) const;

    [[nodiscard]] std::chrono::steady_clock::time_point nextRtuFrameBoundary() const;

    std::optional<QByteArray> tryPopRtuResponseFrame(
        std::chrono::steady_clock::time_point now);

    [[nodiscard]] bool hasExceededDropLimit() const;
    [[nodiscard]] int droppedInvalidBytes() const;

    static std::chrono::microseconds calculateInterFrameDelay(const base::ModbusConfig& config);
    static std::chrono::microseconds calculateInterCharacterDelay(const base::ModbusConfig& config);
    static std::chrono::microseconds calculateFrameDuration(const base::ModbusConfig& config, qsizetype frameBytes);

private:
    static double stopBitsToCount(int stopBits);
    static int parityBitCount(int parity);
    static int bitsPerCharacter(const base::ModbusConfig& config);

    void processTcpBuffer();
    void processAsciiBuffer();
    void processRtuBuffer(std::chrono::steady_clock::time_point now);

    modbus::base::ModbusMode mode_;
    base::ModbusConfig config_;
    QByteArray buffer_;
    std::deque<QByteArray> completedFrames_;
    std::chrono::steady_clock::time_point lastByteReceivedAt_;
    int droppedInvalidBytes_ = 0;
};

} // namespace modbus::session
