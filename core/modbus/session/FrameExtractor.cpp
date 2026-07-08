/**
 * @file FrameExtractor.cpp
 * @brief Implementation of FrameExtractor.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FrameExtractor.h"
#include "../../Config.h"
#include "../base/ModbusProtocolChecks.h"
#include <spdlog/spdlog.h>
#include <cmath>

namespace modbus::session {

// ---------------------------------------------------------------------------
// Static helpers – RTU timing
// ---------------------------------------------------------------------------

double FrameExtractor::stopBitsToCount(int stopBits)
{
    switch (stopBits) {
    case 2:  return 2.0;
    case 3:  return 1.5;
    default: return 1.0;
    }
}

int FrameExtractor::parityBitCount(int parity)
{
    return parity == 0 ? 0 : 1;
}

int FrameExtractor::bitsPerCharacter(const base::ModbusConfig& config)
{
    const int startBits = 1;
    return static_cast<int>(std::ceil(
        static_cast<double>(startBits) +
        static_cast<double>(config.dataBits) +
        static_cast<double>(parityBitCount(config.parity)) +
        stopBitsToCount(config.stopBits)));
}

std::chrono::microseconds FrameExtractor::calculateInterFrameDelay(const base::ModbusConfig& config)
{
    if (config.interFrameDelayUs > 0) {
        return std::chrono::microseconds(config.interFrameDelayUs);
    }
    if (config.baudRate <= 0) {
        return std::chrono::microseconds(1750);
    }
    if (config.baudRate > 19200) {
        return std::chrono::microseconds(1750);
    }

    const double charTimeUs =
        (static_cast<double>(bitsPerCharacter(config)) * 1000000.0) /
        static_cast<double>(config.baudRate);
    return std::chrono::microseconds(
        static_cast<long long>(std::ceil(3.5 * charTimeUs)));
}

std::chrono::microseconds FrameExtractor::calculateInterCharacterDelay(const base::ModbusConfig& config)
{
    if (config.baudRate <= 0) {
        return std::chrono::microseconds(750);
    }
    if (config.baudRate > 19200) {
        return std::chrono::microseconds(750);
    }

    const double charTimeUs =
        (static_cast<double>(bitsPerCharacter(config)) * 1000000.0) /
        static_cast<double>(config.baudRate);
    return std::chrono::microseconds(
        static_cast<long long>(std::ceil(1.5 * charTimeUs)));
}

std::chrono::microseconds FrameExtractor::calculateFrameDuration(
    const base::ModbusConfig& config, qsizetype frameBytes)
{
    if (config.baudRate <= 0 || frameBytes <= 0) {
        return std::chrono::microseconds::zero();
    }

    const double totalBits =
        static_cast<double>(bitsPerCharacter(config)) *
        static_cast<double>(frameBytes);
    const double frameUs =
        (totalBits * 1000000.0) / static_cast<double>(config.baudRate);
    return std::chrono::microseconds(
        static_cast<long long>(std::ceil(frameUs)));
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

FrameExtractor::FrameExtractor(modbus::base::ModbusMode mode, int baudRate)
    : mode_(mode)
{
    config_.baudRate = baudRate;
}

// ---------------------------------------------------------------------------
// feed() – called from onDataReceived
// ---------------------------------------------------------------------------

void FrameExtractor::feed(QByteArrayView data)
{
    if (data.isEmpty()) {
        return;
    }

    const auto now = std::chrono::steady_clock::now();

    if (mode_ == base::ModbusMode::RTU) {
        if (!buffer_.isEmpty() &&
            lastByteReceivedAt_ != std::chrono::steady_clock::time_point{}) {
            const auto silentInterval = now - lastByteReceivedAt_;
            const auto interFrameDelay = calculateInterFrameDelay(config_);

            if (silentInterval >= interFrameDelay) {
                completedFrames_.push_back(buffer_);
                buffer_.clear();
            } else {
                const auto interCharDelay = calculateInterCharacterDelay(config_);
                if (silentInterval > interCharDelay) {
                    spdlog::warn(
                        "FrameExtractor: discarding RTU fragment after "
                        "inter-character gap violation");
                    buffer_.clear();
                }
            }
        }

        buffer_.append(data);

        if (buffer_.size() > config::Modbus::kMaxAduSize) {
            spdlog::error(
                "FrameExtractor: RTU buffer exceeded {} bytes limit, clearing",
                config::Modbus::kMaxAduSize);
            buffer_.clear();
        }

        lastByteReceivedAt_ = now;
    } else if (mode_ == base::ModbusMode::ASCII) {
        buffer_.append(data);
        if (buffer_.size() > config::Modbus::kMaxAduSize * 2) {
            spdlog::error(
                "FrameExtractor: ASCII buffer exceeded {} bytes limit, dropping oldest bytes",
                config::Modbus::kMaxAduSize * 2);
            buffer_.remove(0, buffer_.size() - config::Modbus::kMaxAduSize * 2);
        }
        processAsciiBuffer();
    } else {
        buffer_.append(data);
        if (buffer_.size() > config::Modbus::kMaxTcpBufferedBytes) {
            spdlog::error(
                "FrameExtractor: TCP buffer exceeded {} bytes limit, "
                "dropping oldest bytes",
                config::Modbus::kMaxTcpBufferedBytes);
            buffer_.remove(0,
                buffer_.size() - config::Modbus::kMaxTcpBufferedBytes);
        }
        processTcpBuffer();
    }
}

// ---------------------------------------------------------------------------
// processTcpBuffer() – extract complete TCP frames via MBAP header parsing
// ---------------------------------------------------------------------------

void FrameExtractor::processTcpBuffer()
{
    while (!buffer_.isEmpty()) {
        const int integrity = base::inspectTcpAdu(buffer_);
        if (integrity > 0) {
            if (integrity > config::Modbus::kMaxAduSize) {
                spdlog::error(
                    "FrameExtractor: TCP frame size {} exceeds ADPU limit {}, discarding",
                    integrity, config::Modbus::kMaxAduSize);
                buffer_.remove(0, integrity);
                ++droppedInvalidBytes_;
                continue;
            }
            if (buffer_.size() >= integrity) {
                completedFrames_.push_back(buffer_.left(integrity));
                buffer_.remove(0, integrity);
                droppedInvalidBytes_ = 0;
            }
        } else if (integrity == -1) {
            spdlog::warn(
                "FrameExtractor: invalid TCP MBAP header, dropping first byte "
                "[{}] of buffer totalDropped={}",
                static_cast<int>(static_cast<uint8_t>(buffer_.at(0))),
                droppedInvalidBytes_ + 1);
            buffer_.remove(0, 1);
            ++droppedInvalidBytes_;
            if (droppedInvalidBytes_ > config::Modbus::kMaxDroppedInvalidBytes) {
                spdlog::error(
                    "FrameExtractor: TCP invalid byte limit exceeded ({}), "
                    "clearing buffer",
                    droppedInvalidBytes_);
                buffer_.clear();
                break;
            }
        } else {
            break;
        }
    }
}

void FrameExtractor::processAsciiBuffer()
{
    while (!buffer_.isEmpty()) {
        const int startIndex = buffer_.indexOf(':');
        if (startIndex < 0) {
            droppedInvalidBytes_ += buffer_.size();
            buffer_.clear();
            break;
        }
        if (startIndex > 0) {
            droppedInvalidBytes_ += startIndex;
            buffer_.remove(0, startIndex);
        }

        const qsizetype terminatorIndex = buffer_.indexOf("\r\n");
        if (terminatorIndex < 0) {
            break;
        }

        const int frameLength = static_cast<int>(terminatorIndex + 2);
        const QByteArray candidate = buffer_.left(frameLength);
        const int integrity = base::inspectAsciiAdu(candidate);
        if (integrity > 0) {
            completedFrames_.push_back(candidate);
            buffer_.remove(0, frameLength);
            droppedInvalidBytes_ = 0;
            continue;
        }

        spdlog::warn(
            "FrameExtractor: invalid ASCII frame header/body, dropping start delimiter");
        buffer_.remove(0, 1);
        ++droppedInvalidBytes_;
        if (droppedInvalidBytes_ > config::Modbus::kMaxDroppedInvalidBytes) {
            spdlog::error(
                "FrameExtractor: ASCII invalid byte limit exceeded ({}), clearing buffer",
                droppedInvalidBytes_);
            buffer_.clear();
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// processRtuBuffer() – extract RTU frame when silence period elapsed
// ---------------------------------------------------------------------------

void FrameExtractor::processRtuBuffer(std::chrono::steady_clock::time_point now)
{
    if (mode_ != base::ModbusMode::RTU) {
        return;
    }
    if (!completedFrames_.empty()) {
        return;
    }
    if (buffer_.isEmpty()) {
        return;
    }
    if (!isRtuFrameReadyToParse(now)) {
        return;
    }

    if (buffer_.size() > config::Modbus::kMaxAduSize) {
        spdlog::error(
            "FrameExtractor: RTU frame size {} exceeds ADPU limit {}, discarding",
            buffer_.size(), config::Modbus::kMaxAduSize);
        buffer_.clear();
        lastByteReceivedAt_ = std::chrono::steady_clock::time_point{};
        return;
    }

    completedFrames_.push_back(buffer_);
    buffer_.clear();
    lastByteReceivedAt_ = std::chrono::steady_clock::time_point{};
}

// ---------------------------------------------------------------------------
// Queue access
// ---------------------------------------------------------------------------

bool FrameExtractor::hasCompleteFrame() const
{
    return !completedFrames_.empty();
}

std::optional<QByteArray> FrameExtractor::popFrame()
{
    if (completedFrames_.empty()) {
        return std::nullopt;
    }
    QByteArray frame = completedFrames_.front();
    completedFrames_.pop_front();
    return frame;
}

void FrameExtractor::reset()
{
    buffer_.clear();
    completedFrames_.clear();
    lastByteReceivedAt_ = {};
    droppedInvalidBytes_ = 0;
}

void FrameExtractor::setConfig(const base::ModbusConfig& config)
{
    mode_ = config.mode;
    config_ = config;
    reset();
}

qsizetype FrameExtractor::bufferSize() const
{
    return buffer_.size();
}

bool FrameExtractor::hasExceededDropLimit() const
{
    return droppedInvalidBytes_ > config::Modbus::kMaxDroppedInvalidBytes;
}

int FrameExtractor::droppedInvalidBytes() const
{
    return droppedInvalidBytes_;
}

// ---------------------------------------------------------------------------
// RTU query methods – used by sendRequestInternal wait loop
// ---------------------------------------------------------------------------

bool FrameExtractor::isRtuFrameReadyToParse(
    std::chrono::steady_clock::time_point now) const
{
    if (mode_ != base::ModbusMode::RTU && mode_ != base::ModbusMode::ASCII) {
        return false;
    }
    if (!completedFrames_.empty()) {
        return true;
    }
    if (buffer_.isEmpty()) {
        return false;
    }
    if (lastByteReceivedAt_ == std::chrono::steady_clock::time_point{}) {
        return false;
    }
    return now >= nextRtuFrameBoundary();
}

std::chrono::steady_clock::time_point FrameExtractor::nextRtuFrameBoundary() const
{
    if (lastByteReceivedAt_ == std::chrono::steady_clock::time_point{}) {
        return std::chrono::steady_clock::time_point{};
    }
    return lastByteReceivedAt_ + calculateInterFrameDelay(config_);
}

std::optional<QByteArray> FrameExtractor::tryPopRtuResponseFrame(
    std::chrono::steady_clock::time_point now)
{
    if (mode_ != base::ModbusMode::RTU && mode_ != base::ModbusMode::ASCII) {
        return std::nullopt;
    }

    if (!completedFrames_.empty()) {
        QByteArray frame = completedFrames_.front();
        completedFrames_.pop_front();
        return frame;
    }

    if (mode_ == base::ModbusMode::RTU) {
        processRtuBuffer(now);
    }

    if (!completedFrames_.empty()) {
        QByteArray frame = completedFrames_.front();
        completedFrames_.pop_front();
        return frame;
    }

    return std::nullopt;
}

} // namespace modbus::session
