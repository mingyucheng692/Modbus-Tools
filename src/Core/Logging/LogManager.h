#pragma once
#include <memory>
#include <spdlog/spdlog.h>
#include "QtLogSink.h"

class LogManager {
public:
    static LogManager& instance();
    void init();
    std::shared_ptr<spdlog::logger> getSystemLogger();
    QtLogSinkBase* getQtSinkBase();

private:
    LogManager() = default;
    std::shared_ptr<spdlog::logger> systemLogger_;
    QtLogSinkBase* qtSinkBase_ = nullptr;
};
