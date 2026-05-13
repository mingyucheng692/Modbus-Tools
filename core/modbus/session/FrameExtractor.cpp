/**
 * @file FrameExtractor.cpp
 * @brief Implementation of FrameExtractor.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FrameExtractor.h"
#include "AppConstants.h"
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
// extract() – called from onDataReceived
// ---------------------------------------------------------------------------

bool FrameExtractor::extract(QByteArray& buffer, QByteArrayView newData,
    std::chrono::steady_clock::time_point now)
{
    if (newData.isEmpty()) {
        return false;
    }

    if (mode_ == base::ModbusMode::RTU) {
        // --- inter-frame silence detection ---
        if (!buffer.isEmpty() &&
            lastByteReceivedAt_ != std::chrono::steady_clock::time_point{}) {
            const auto silentInterval = now - lastByteReceivedAt_;
            const auto interFrameDelay = calculateInterFrameDelay(config_);

            if (silentInterval >= interFrameDelay) {
                completedFrames_.push_back(buffer);
                buffer.clear();
            } else {
                const auto interCharDelay = calculateInterCharacterDelay(config_);
                if (silentInterval > interCharDelay) {
                    spdlog::warn(
                        "FrameExtractor: discarding RTU fragment after "
                        "inter-character gap violation");
                    buffer.clear();
                }
            }
        }

        // --- append new data ---
        buffer.append(newData);

        // --- buffer overflow ---
        if (buffer.size() > app::constants::Values::Modbus::kMaxRtuBufferedBytes) {
            spdlog::warn(
                "FrameExtractor: RTU buffer exceeded {} bytes limit, clearing",
                app::constants::Values::Modbus::kMaxRtuBufferedBytes);
            buffer.clear();
        }

        lastByteReceivedAt_ = now;
    } else {
        // --- TCP ---
        buffer.append(newData);
        if (buffer.size() > app::constants::Values::Modbus::kMaxTcpBufferedBytes) {
            spdlog::warn(
                "FrameExtractor: TCP buffer exceeded {} bytes limit, "
                "dropping oldest bytes",
                app::constants::Values::Modbus::kMaxTcpBufferedBytes);
            buffer.remove(0,
                buffer.size() - app::constants::Values::Modbus::kMaxTcpBufferedBytes);
        }
    }
    return true;
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
    completedFrames_.clear();
    lastByteReceivedAt_ = {};
}

void FrameExtractor::setConfig(const base::ModbusConfig& config)
{
    mode_ = config.mode;
    config_ = config;
    reset();
}

// ---------------------------------------------------------------------------
// RTU query methods – used by sendRequestInternal wait loop
// ---------------------------------------------------------------------------

bool FrameExtractor::isRtuFrameReadyToParse(
    std::chrono::steady_clock::time_point now,
    const QByteArray& buffer) const
{
    if (mode_ != base::ModbusMode::RTU) {
        return false;
    }
    if (!completedFrames_.empty()) {
        return true;
    }
    if (buffer.isEmpty()) {
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

bool FrameExtractor::tryPopRtuResponseFrame(QByteArray& buffer,
    std::chrono::steady_clock::time_point now,
    QByteArray& frame)
{
    if (mode_ != base::ModbusMode::RTU) {
        return false;
    }

    if (!completedFrames_.empty()) {
        frame = completedFrames_.front();
        completedFrames_.pop_front();
        return true;
    }

    if (buffer.isEmpty() || buffer.size() < 5 ||
        !isRtuFrameReadyToParse(now, buffer)) {
        return false;
    }

    frame = std::move(buffer);
    buffer.clear();
    lastByteReceivedAt_ = std::chrono::steady_clock::time_point{};
    return true;
}

} // namespace modbus::session