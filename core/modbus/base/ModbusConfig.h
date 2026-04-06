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
    uint8_t slaveId = app::constants::Constants::Modbus::kDefaultSlaveId;

    // --- 连接参数 ---
    // Serial (RTU)
    QString portName;
    int baudRate = app::constants::Constants::Serial::kDefaultBaudRate;
    int dataBits = app::constants::Constants::Serial::kDefaultDataBits;
    int stopBits = app::constants::Constants::Serial::kDefaultStopBits;
    int parity = app::constants::Constants::Serial::kDefaultParityValue; // 0:No, 2:Even, 3:Odd (QSerialPort::Parity)

    // TCP
    QString ipAddress = QString::fromLatin1(app::constants::Constants::Network::kLoopbackAddress);
    int port = app::constants::Constants::Network::kDefaultModbusTcpPort;
    // ----------------

    // 超时时间 (毫秒)
    int timeoutMs = app::constants::Constants::Modbus::kDefaultTimeoutMs;

    // 重试次数
    int retries = 0;

    // 重试间隔 (毫秒)
    int retryIntervalMs = app::constants::Constants::Modbus::kDefaultRetryIntervalMs;

    // 帧间隔 (RTU only, micro-seconds or char times)
    // 通常由驱动层处理，但在某些应用层实现中可能需要
    int interFrameDelayUs = 0;
};

} // namespace modbus::base
