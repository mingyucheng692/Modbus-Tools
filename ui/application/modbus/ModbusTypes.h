/**
 * @file ModbusTypes.h
 * @brief Strong type definitions for Modbus application layer.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <chrono>
#include <cstdint>
#include <optional>
#include "modbus/base/ModbusFrame.h"
#include "modbus/base/ModbusConfig.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "infra/io/SerialConfig.h"
#include "../../../../core/Config.h"

namespace ui::application::modbus {

struct PollSpec {
    uint8_t functionCode = 0;
    uint16_t startAddress = 0;
    uint16_t quantity = 0;
    uint8_t slaveId = static_cast<uint8_t>(config::Modbus::kDefaultSlaveId);
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

enum class SessionMode {
    Tcp,
    Rtu,
    Ascii
};

enum class TransportUiMode {
    Tcp,
    Rtu,
    Ascii
};

struct ModbusModeDescriptor {
    SessionMode sessionMode = SessionMode::Tcp;
    ::modbus::base::ModbusMode modbusMode = ::modbus::base::ModbusMode::TCP;
    ::modbus::core::parser::ProtocolType protocolType = ::modbus::core::parser::ProtocolType::Tcp;
    TransportUiMode transportUiMode = TransportUiMode::Tcp;
    const char* settingsPrefix = "modbus/tcp";
    const char* logName = "TCP";
    bool usesSerialConnection = false;
    bool usesUnitIdLabel = true;
};

struct ModbusConnectionSpec {
    ::modbus::base::ModbusConfig config;
    std::optional<io::SerialConfig> serialConfig;
};

[[nodiscard]] constexpr SessionMode sessionModeFor(::modbus::base::ModbusMode mode) {
    switch (mode) {
    case ::modbus::base::ModbusMode::TCP:
        return SessionMode::Tcp;
    case ::modbus::base::ModbusMode::RTU:
        return SessionMode::Rtu;
    case ::modbus::base::ModbusMode::ASCII:
        return SessionMode::Ascii;
    }

    return SessionMode::Tcp;
}

[[nodiscard]] constexpr ModbusModeDescriptor modeDescriptor(SessionMode mode) {
    switch (mode) {
    case SessionMode::Tcp:
        return ModbusModeDescriptor{
            SessionMode::Tcp,
            ::modbus::base::ModbusMode::TCP,
            ::modbus::core::parser::ProtocolType::Tcp,
            TransportUiMode::Tcp,
            "modbus/tcp",
            "TCP",
            false,
            true
        };
    case SessionMode::Rtu:
        return ModbusModeDescriptor{
            SessionMode::Rtu,
            ::modbus::base::ModbusMode::RTU,
            ::modbus::core::parser::ProtocolType::Rtu,
            TransportUiMode::Rtu,
            "modbus/rtu",
            "RTU",
            true,
            false
        };
    case SessionMode::Ascii:
        return ModbusModeDescriptor{
            SessionMode::Ascii,
            ::modbus::base::ModbusMode::ASCII,
            ::modbus::core::parser::ProtocolType::Ascii,
            TransportUiMode::Ascii,
            "modbus/ascii",
            "ASCII",
            true,
            false
        };
    }

    return modeDescriptor(SessionMode::Tcp);
}

[[nodiscard]] constexpr ModbusModeDescriptor modeDescriptor(::modbus::base::ModbusMode mode) {
    return modeDescriptor(sessionModeFor(mode));
}

} // namespace ui::application::modbus
