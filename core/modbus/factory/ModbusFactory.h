#pragma once

#include "IModbusFactory.h"
#include "../../io/IChannel.h"
#include "../transport/ITransport.h"

namespace modbus::factory {

// 具体工厂实现
class ModbusFactory : public IModbusFactory {
public:
    std::unique_ptr<dispatch::ModbusWorker> createWorker(const base::ModbusConfig& config) override;

private:
    std::shared_ptr<io::IChannel> createChannel(const base::ModbusConfig& config);
    std::shared_ptr<transport::ITransport> createTransport(const base::ModbusConfig& config);
};

} // namespace modbus::factory
