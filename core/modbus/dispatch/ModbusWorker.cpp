#include "ModbusWorker.h"
#include <spdlog/spdlog.h>

namespace modbus::dispatch {

ModbusWorker::ModbusWorker(std::shared_ptr<session::IModbusClient> client)
    : client_(std::move(client)) {}

ModbusWorker::~ModbusWorker() {
    stop();
}

void ModbusWorker::start() {
    if (running_) return;
    
    running_ = true;
    queue_.start();
    thread_ = std::thread(&ModbusWorker::loop, this);
    spdlog::info("ModbusWorker started");
}

void ModbusWorker::stop() {
    if (!running_) return;

    running_ = false;
    queue_.stop();
    if (thread_.joinable()) {
        thread_.join();
    }
    spdlog::info("ModbusWorker stopped");
}

std::future<session::ModbusResponse> ModbusWorker::submit(base::Pdu request, int slaveId) {
    // 这里如果 client 支持异步，我们可以直接调用 client->sendRequest
    // 但为了统一管理任务（如优先级、取消等），也可以放入队列
    // 目前简单实现：直接调用 client
    if (!client_) {
        // 返回一个包含错误的 Future
        std::promise<session::ModbusResponse> promise;
        promise.set_value(session::ModbusResponse::Error("No client attached"));
        return promise.get_future();
    }
    return client_->sendRequest(request, slaveId);
}

void ModbusWorker::sendRaw(const QByteArray& data) {
    if (client_) {
        client_->sendRaw(data);
    }
}

void ModbusWorker::loop() {
    while (running_) {
        // 阻塞等待命令
        auto cmdOpt = queue_.pop();
        if (!cmdOpt) {
            // 队列停止且为空，退出循环
            break;
        }

        ModbusCommand& cmd = *cmdOpt;
        
        // 确保连接
        if (!client_->isConnected()) {
             if (!client_->connect()) {
                 cmd.promise.set_value(session::ModbusResponse::Error("Failed to connect"));
                 continue;
             }
        }

        // 执行命令 (Client 内部可能是同步或异步，但这里我们同步等待 Client 的 Future)
        // 实际上 Client::sendRequest 是异步的，但为了保持 Worker 的串行性，我们这里 wait
        try {
            auto clientFuture = client_->sendRequest(cmd.request);
            // 等待结果
            auto response = clientFuture.get(); 
            cmd.promise.set_value(response);
        } catch (const std::exception& e) {
            cmd.promise.set_value(session::ModbusResponse::Error(QString("Exception: %1").arg(e.what())));
        }
    }
}

} // namespace modbus::dispatch
