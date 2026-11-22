// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArray>
#include <QObject>
#include <chrono>
#include <cstdint>
#include <optional>
#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusConfig.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "infra/io/SerialConfig.h"
#include "Config.h"

namespace ui::application::modbus {

using TraceId = quint64;

struct PollSpec {
    uint8_t functionCode = 0;
    uint16_t startAddress = 0;
    uint16_t quantity = 0;
    uint8_t slaveId = static_cast<uint8_t>(config::Modbus::kDefaultSlaveId);
    int intervalMs = config::Polling::kDefaultIntervalMs;
    QByteArray writeData;
};

struct ModbusTimingParams {
    std::chrono::milliseconds timeout{config::Modbus::kDefaultTimeoutMs};
    int retryCount = config::Modbus::kDefaultRetryCount;
    std::chrono::milliseconds retryInterval{config::Modbus::kDefaultRetryIntervalMs};
};

enum class RequestKind {
    Read,
    Write,
    Poll,
    Raw
};

struct ModbusModeDescriptor {
    ::modbus::base::ModbusMode modbusMode = ::modbus::base::ModbusMode::TCP;
    ::modbus::parser::ProtocolType protocolType = ::modbus::parser::ProtocolType::Tcp;
    const char* logName = "TCP";
    bool usesSerialConnection = false;
};

struct ModbusConnectionSpec {
    ::modbus::base::ModbusConfig config;
    std::optional<io::SerialConfig> serialConfig;
};

[[nodiscard]] constexpr ModbusModeDescriptor modeDescriptor(::modbus::base::ModbusMode mode) {
    switch (mode) {
    case ::modbus::base::ModbusMode::TCP:
        return ModbusModeDescriptor{
            ::modbus::base::ModbusMode::TCP,
            ::modbus::parser::ProtocolType::Tcp,
            "TCP",
            false
        };
    case ::modbus::base::ModbusMode::RTU:
        return ModbusModeDescriptor{
            ::modbus::base::ModbusMode::RTU,
            ::modbus::parser::ProtocolType::Rtu,
            "RTU",
            true
        };
    case ::modbus::base::ModbusMode::ASCII:
        return ModbusModeDescriptor{
            ::modbus::base::ModbusMode::ASCII,
            ::modbus::parser::ProtocolType::Ascii,
            "ASCII",
            true
        };
    }

    return modeDescriptor(::modbus::base::ModbusMode::TCP);
}

// UI-layer connection state, projected from core ConnectionStateMachine and channel state.
enum class SessionConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Disconnecting
};

/// Returns a stable, compile-time string representation of a SessionConnectionState.
/// Safe to call from any context (no QObject dependency, constexpr, inline).
[[nodiscard]] constexpr const char* connectionStateName(SessionConnectionState state) noexcept {
    switch (state) {
    case SessionConnectionState::Disconnected:       return "Disconnected";
    case SessionConnectionState::Connecting:         return "Connecting";
    case SessionConnectionState::Connected:          return "Connected";
    case SessionConnectionState::Disconnecting:      return "Disconnecting";
    }
    return "Unknown";
}

} // namespace ui::application::modbus
