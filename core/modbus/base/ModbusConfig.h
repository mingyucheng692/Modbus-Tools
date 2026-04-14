/**
 * @file ModbusConfig.h
 * @brief Header file for ModbusConfig.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "AppConstants.h"
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
    uint8_t slaveId = app::constants::Values::Modbus::kDefaultSlaveId;

    // --- 连接参数 ---
    // Serial (RTU)
    QString portName;
    int baudRate = app::constants::Values::Serial::kDefaultBaudRate;
    int dataBits = app::constants::Values::Serial::kDefaultDataBits;
    int stopBits = app::constants::Values::Serial::kDefaultStopBits;
    int parity = app::constants::Values::Serial::kDefaultParityValue; // 0:No, 2:Even, 3:Odd (QSerialPort::Parity)

    // TCP
    QString ipAddress = QString::fromLatin1(app::constants::Values::Network::kLoopbackAddress);
    int port = app::constants::Values::Network::kDefaultModbusTcpPort;
    // ----------------

    // 超时时间 (毫秒)
    int timeoutMs = app::constants::Values::Modbus::kDefaultTimeoutMs;

    // 重试次数
    int retries = 0;

    // 重试间隔 (毫秒)
    int retryIntervalMs = app::constants::Values::Modbus::kDefaultRetryIntervalMs;

    // 重试退避上限 (毫秒)
    int maxRetryIntervalMs = app::constants::Values::Modbus::kDefaultMaxRetryIntervalMs;

    // 指数退避系数
    double retryBackoffFactor = app::constants::Values::Modbus::kDefaultRetryBackoffFactor;

    // 退避抖动百分比
    int retryJitterPercent = app::constants::Values::Modbus::kDefaultRetryJitterPercent;

    // 自动重连开关
    bool autoReconnect = app::constants::Values::Modbus::kDefaultAutoReconnect;

    // 自动重连基础间隔 (毫秒)
    int reconnectBaseMs = app::constants::Values::Modbus::kDefaultReconnectBaseMs;

    // 自动重连最大间隔 (毫秒)
    int reconnectMaxMs = app::constants::Values::Modbus::kDefaultReconnectMaxMs;

    // 帧间隔 (RTU only, micro-seconds or char times)
    // 通常由驱动层处理，但在某些应用层实现中可能需要
    int interFrameDelayUs = 0;
};

} // namespace modbus::base
