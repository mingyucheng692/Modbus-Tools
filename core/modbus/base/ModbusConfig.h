/**
 * @file ModbusConfig.h
 * @brief Header file for ModbusConfig.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

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

    // 重试次数 (全局默认/回退值，0 表示不自动重试)
    int retries = config::Modbus::kDefaultCoreRetries;

    // 差异化重试：读操作快速失败保刷新率，写操作自动重试保可靠性
    int pollRetries = config::Modbus::kDefaultPollRetries;   ///< Default 0 for periodic polling reads (Fail-Fast)
    int writeRetries = config::Modbus::kDefaultWriteRetries; ///< Default 1 for control writes (Reliability)

    // 重试间隔 (毫秒)
    int retryIntervalMs = config::Modbus::kDefaultRetryIntervalMs;

    // 重试退避上限 (毫秒)
    int maxRetryIntervalMs = config::Modbus::kDefaultMaxRetryIntervalMs;

    // 指数退避系数
    double retryBackoffFactor = config::Modbus::kDefaultRetryBackoffFactor;

    // 退避抖动百分比
    int retryJitterPercent = config::Modbus::kDefaultRetryJitterPercent;
    // 半开检测：会话报告已连接时连续请求超时的驱逐阈值。
    // 达到阈值即判定半开连接（对端消失但无 RST）并拆除会话；
    // 0 表示禁用。每次成功请求会将计数清零。
    int unresponsiveThreshold = config::Modbus::kDefaultUnresponsiveThreshold;

    // 帧间隔 (RTU only, micro-seconds or char times)
    // 通常由驱动层处理，但在某些应用层实现中可能需要
    int interFrameDelayUs = 0;
};

} // namespace modbus::base
