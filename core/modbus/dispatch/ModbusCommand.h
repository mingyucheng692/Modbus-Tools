#pragma once

#include <functional>
#include <future>
#include "../base/ModbusFrame.h"
#include "../session/IModbusClient.h"

namespace modbus::dispatch {

// 命令对象：封装一个请求和它的 Promise
struct ModbusCommand {
    base::Pdu request;
    std::promise<session::ModbusResponse> promise;

    ModbusCommand(base::Pdu req) : request(req) {}
    
    // 移动构造和赋值，因为 promise 不可复制
    ModbusCommand(ModbusCommand&&) = default;
    ModbusCommand& operator=(ModbusCommand&&) = default;
    
    // 禁用复制
    ModbusCommand(const ModbusCommand&) = delete;
    ModbusCommand& operator=(const ModbusCommand&) = delete;
};

} // namespace modbus::dispatch
