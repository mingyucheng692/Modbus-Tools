/**
 * @file ModbusConfig.h
 * @brief Header file for ModbusConfig.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "Config.h"
#include <cstdint>
#include <QString>

namespace modbus::base {

enum class ModbusMode {
    RTU,
    TCP,
    ASCII // 暂不实现，预留
};

struct ModbusConfig {
    ModbusMode mode = ModbusMode::RTU;
    
    // 从站 ID (Unit ID)
    uint8_t slaveId = config::Modbus::kDefaultSlaveId;

    // --- 连接参数 ---
    // Serial (RTU)
    QString portName;
    int baudRate = config::Serial::kDefaultBaudRate;
    int dataBits = config::Serial::kDefaultDataBits;
    int stopBits = config::Serial::kDefaultStopBits;
    int parity = config::Serial::kDefaultParityValue; // 0:No, 2:Even, 3:Odd (QSerialPort::Parity)

    // TCP
    QString ipAddress = QString::fromLatin1(config::Network::kDefaultDeviceAddress);
    int port = config::Network::kDefaultModbusTcpPort;
    // ----------------

    // 超时时间 (毫秒)
    int timeoutMs = config::Modbus::kDefaultTimeoutMs;

    // 重试次数
    int retries = 0;

    // 重试间隔 (毫秒)
    int retryIntervalMs = config::Modbus::kDefaultRetryIntervalMs;

    // 重试退避上限 (毫秒)
    int maxRetryIntervalMs = config::Modbus::kDefaultMaxRetryIntervalMs;

    // 指数退避系数
    double retryBackoffFactor = config::Modbus::kDefaultRetryBackoffFactor;

    // 退避抖动百分比
    int retryJitterPercent = config::Modbus::kDefaultRetryJitterPercent;

    // 自动重连开关
    bool autoReconnect = config::Modbus::kDefaultAutoReconnect;

    // 自动重连基础间隔 (毫秒)
    int reconnectBaseMs = config::Modbus::kDefaultReconnectBaseMs;

    // 自动重连最大间隔 (毫秒)
    int reconnectMaxMs = config::Modbus::kDefaultReconnectMaxMs;

    // 帧间隔 (RTU only, micro-seconds or char times)
    // 通常由驱动层处理，但在某些应用层实现中可能需要
    int interFrameDelayUs = 0;
};

} // namespace modbus::base
