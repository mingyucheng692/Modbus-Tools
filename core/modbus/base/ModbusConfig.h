#pragma once

#include <cstdint>

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

    // 通信参数 (适用于 TCP 或 串口配置)
    // 注意：具体串口参数(波特率等)或IP端口由 io::IChannel 配置负责，此处仅负责 Modbus 协议层参数
    
    // 超时时间 (毫秒)
    int timeoutMs = 1000;

    // 重试次数
    int retries = 3;

    // 帧间隔 (RTU only, micro-seconds or char times)
    // 通常由驱动层处理，但在某些应用层实现中可能需要
    int interFrameDelayUs = 0;
};

} // namespace modbus::base
