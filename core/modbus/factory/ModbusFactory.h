/**
 * @file ModbusFactory.h
 * @brief Header file for ModbusFactory.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <memory>
#include <QThread>
#include "infra/io/IChannel.h"
#include "../transport/ITransport.h"
#include "../dispatch/ModbusWorker.h"
#include "../base/ModbusConfig.h"

namespace modbus::session { class ModbusClient; }

namespace modbus::factory {

struct ModbusStack {
    std::shared_ptr<io::IChannel> channel;
    std::shared_ptr<session::ModbusClient> client;
    std::shared_ptr<QThread> ioThread;
    std::shared_ptr<QThread> thread;
    std::shared_ptr<dispatch::ModbusWorker> worker;
};

// 工厂自由函数：根据配置创建并装配完整的 Modbus 协议栈。
ModbusStack createStack(const base::ModbusConfig& config);

} // namespace modbus::factory
