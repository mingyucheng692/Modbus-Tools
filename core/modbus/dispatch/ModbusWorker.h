#pragma once

#include "CommandQueue.h"
#include "../session/IModbusClient.h"
#include <thread>
#include <atomic>
#include <memory>

namespace modbus::dispatch {

class ModbusWorker {
public:
    explicit ModbusWorker(std::shared_ptr<session::IModbusClient> client);
    ~ModbusWorker();

    void start();
    void stop();

    // 提交任务，返回 Future
    std::future<session::ModbusResponse> submit(base::Pdu request, int slaveId = -1);

    // 发送原始数据
    void sendRaw(const QByteArray& data);

private:
    void loop();

    std::shared_ptr<session::IModbusClient> client_;
    CommandQueue queue_;
    std::thread thread_;
    std::atomic<bool> running_{false};
};

} // namespace modbus::dispatch
