#pragma once

#include <memory>
#include "../dispatch/ModbusWorker.h"
#include "../session/IModbusClient.h"
#include "../base/ModbusConfig.h"
#include "../../io/IChannel.h"
#include <QThread>

namespace modbus::factory {

// 抽象工厂接口
struct ModbusStack {
    std::shared_ptr<io::IChannel> channel;
    std::shared_ptr<session::IModbusClient> client;
    std::shared_ptr<QThread> thread;
    std::shared_ptr<dispatch::ModbusWorker> worker;
};

class IModbusFactory {
public:
    virtual ~IModbusFactory() = default;

    virtual ModbusStack createStack(const base::ModbusConfig& config) = 0;
};

} // namespace modbus::factory
