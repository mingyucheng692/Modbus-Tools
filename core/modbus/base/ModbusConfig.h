// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "../../Config.h"
#include <cstdint>
#include <QString>

namespace modbus::base {

enum class ModbusMode {
    RTU,
    TCP,
    ASCII // Serial ASCII framing with LRC check
};

// Role: Configuration aggregate parameterising Modbus protocol stack, transport, and session policies.
// Thread-safety: Thread-compatible (plain value struct).
// Ownership: Value semantics; passed by copy or const reference.
struct ModbusConfig {
    ModbusMode mode = ModbusMode::RTU;

    uint8_t slaveId = config::Modbus::kDefaultSlaveId; // [1..247, 0=broadcast]

    // Serial (RTU/ASCII)
    QString portName;
    int baudRate = config::Serial::kDefaultBaudRate;
    int dataBits = config::Serial::kDefaultDataBits;
    int stopBits = config::Serial::kDefaultStopBits;
    int parity = config::Serial::kDefaultParityValue; // 0:No, 2:Even, 3:Odd (QSerialPort::Parity)

    // TCP
    QString ipAddress = QString::fromLatin1(config::Network::kDefaultDeviceAddress);
    int port = config::Network::kDefaultModbusTcpPort;

    int timeoutMs = config::Modbus::kDefaultTimeoutMs; // [ms]

    // Global default retry count [0=no retry]
    int retries = config::Modbus::kDefaultCoreRetries;

    // Differentiated retries: poll reads fail fast, control writes retry for reliability
    int pollRetries = config::Modbus::kDefaultPollRetries;   // [count, 0=fail-fast]
    int writeRetries = config::Modbus::kDefaultWriteRetries; // [count]

    int retryIntervalMs = config::Modbus::kDefaultRetryIntervalMs;       // [ms]
    int maxRetryIntervalMs = config::Modbus::kDefaultMaxRetryIntervalMs; // [ms]
    double retryBackoffFactor = config::Modbus::kDefaultRetryBackoffFactor; // Multiplier
    int retryJitterPercent = config::Modbus::kDefaultRetryJitterPercent;    // [%]

    // Half-open eviction threshold: consecutive timeouts required while connected before tearing down link.
    // 0 disables eviction; reset to 0 upon every successful request.
    int unresponsiveThreshold = config::Modbus::kDefaultUnresponsiveThreshold;

    // Inter-frame silence interval for RTU [us]. 0 delegates timing to driver/hardware.
    int interFrameDelayUs = 0;
};

} // namespace modbus::base
