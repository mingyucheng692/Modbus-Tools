#pragma once

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
    uint8_t slaveId = 1;

    // --- 连接参数 ---
    // Serial (RTU)
    QString portName;
    int baudRate = 9600;
    int dataBits = 8;
    int stopBits = 1;
    int parity = 0; // 0:No, 2:Even, 3:Odd (QSerialPort::Parity)

    // TCP
    QString ipAddress = "127.0.0.1";
    int port = 502;
    // ----------------

    // 超时时间 (毫秒)
    int timeoutMs = 1000;

    // 重试次数
    int retries = 0;

    // 重试间隔 (毫秒)
    int retryIntervalMs = 100;

    // 帧间隔 (RTU only, micro-seconds or char times)
    // 通常由驱动层处理，但在某些应用层实现中可能需要
    int interFrameDelayUs = 0;
};

} // namespace modbus::base
