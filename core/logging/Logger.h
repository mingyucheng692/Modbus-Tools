#pragma once

#include <QString>
#include <memory>
#include <spdlog/spdlog.h>

namespace logging {

void Init(const QString& logDir);
void Shutdown();
void Flush();
void SetLevel(spdlog::level::level_enum level);
std::shared_ptr<spdlog::logger> Get();

}
