/**
 * @file Logger.h
 * @brief Header file for Logger.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QString>
#include <memory>
#include <spdlog/spdlog.h>

#if defined(MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS) && MODBUS_TOOLS_ENABLE_VERBOSE_RUNTIME_LOGS
#define MODBUS_TOOLS_VERBOSE_INFO(...) spdlog::info(__VA_ARGS__)
#else
#define MODBUS_TOOLS_VERBOSE_INFO(...) ((void)0)
#endif

namespace logging {

void Init(const QString& logDir);
void Shutdown();
void Flush();
void SetLevel(spdlog::level::level_enum level);
std::shared_ptr<spdlog::logger> Get();

}
