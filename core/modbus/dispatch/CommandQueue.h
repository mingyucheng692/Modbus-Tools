#pragma once

#include "ModbusCommand.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace modbus::dispatch {

class CommandQueue {
public:
    // 推入命令
    void push(ModbusCommand command) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(command));
        cv_.notify_one();
    }

    // 阻塞弹出一个命令（用于 Worker 线程）
    // 返回 nullopt 表示队列已关闭且为空
    std::optional<ModbusCommand> pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
        
        if (stopped_ && queue_.empty()) {
            return std::nullopt;
        }
        
        if (queue_.empty()) {
             return std::nullopt; // Should not happen if logic is correct
        }

        ModbusCommand cmd = std::move(queue_.front());
        queue_.pop();
        return cmd;
    }

    // 停止队列（唤醒所有等待者）
    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
        cv_.notify_all();
    }
    
    // 恢复队列
    void start() {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = false;
    }

private:
    std::queue<ModbusCommand> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
};

} // namespace modbus::dispatch
