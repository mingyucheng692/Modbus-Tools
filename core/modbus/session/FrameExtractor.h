/**
 * @file FrameExtractor.h
 * @brief Header file for FrameExtractor.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../base/ModbusConfig.h"
#include "../base/ModbusTypes.h"
#include <QByteArray>
#include <deque>
#include <chrono>
#include <optional>

namespace modbus::session {

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
