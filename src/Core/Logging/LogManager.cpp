#include "LogManager.h"
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>

LogManager& LogManager::instance() {
    static LogManager instance;
    return instance;
}

void LogManager::init() {
    // 1. Initialize Thread Pool for Async Logging
    spdlog::init_thread_pool(8192, 1);

    // 2. Create Sinks
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    // Manage lifecycle of qtSinkBase properly in real app, here simplified
    qtSinkBase_ = new QtLogSinkBase(); 
    auto qt_sink = std::make_shared<QtLogSink<std::mutex>>(qtSinkBase_);

    std::vector<spdlog::sink_ptr> sinks {console_sink, qt_sink};

    // 3. Create Logger
    systemLogger_ = std::make_shared<spdlog::async_logger>("sys", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    
    spdlog::register_logger(systemLogger_);
    spdlog::set_default_logger(systemLogger_);
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%l%$] %v");
    spdlog::flush_every(std::chrono::seconds(1));
}

std::shared_ptr<spdlog::logger> LogManager::getSystemLogger() {
    return systemLogger_;
}

QtLogSinkBase* LogManager::getQtSinkBase() {
    return qtSinkBase_;
}
