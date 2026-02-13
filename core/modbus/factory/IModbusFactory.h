#pragma once

#include <memory>
#include "../dispatch/ModbusWorker.h"
#include "../base/ModbusConfig.h"

namespace modbus::factory {

// 抽象工厂接口
class IModbusFactory {
public:
    virtual ~IModbusFactory() = default;

    // 创建一个完全配置好的 Modbus Worker
    // 包含：Channel (Serial/TCP) + Transport (RTU/TCP) + Client + Worker
    virtual std::unique_ptr<dispatch::ModbusWorker> createWorker(const base::ModbusConfig& config) = 0;
};

} // namespace modbus::factory
