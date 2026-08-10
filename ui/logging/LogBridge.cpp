/**
 * @file LogBridge.cpp
 * @brief Implementation of strategic TrafficEvent to spdlog bridge.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "LogBridge.h"
#include "../common/TrafficEvent.h"
#include <spdlog/spdlog.h>

namespace ui::logging {

void relay(const ui::common::TrafficEvent& event)
{
    const bool isTraceableRequest = event.traceId != 0;
    if (!isTraceableRequest && event.level != ui::common::TrafficEventLevel::Error) {
        return;
    }

    switch (event.level) {
    case ui::common::TrafficEventLevel::Info:
        if (event.traceId != 0) {
            spdlog::info("Traffic trace_id={} type={} dir={} summary={}",
                         static_cast<unsigned long long>(event.traceId),
                         ui::common::toString(event.requestType),
                         ui::common::toString(event.direction),
                         event.summary.toStdString());
        }
        break;
    case ui::common::TrafficEventLevel::Warning:
        if (event.traceId != 0) {
            spdlog::warn("Traffic trace_id={} type={} dir={} summary={}",
                         static_cast<unsigned long long>(event.traceId),
                         ui::common::toString(event.requestType),
                         ui::common::toString(event.direction),
                         event.summary.toStdString());
        }
        break;
    case ui::common::TrafficEventLevel::Error:
        if (event.traceId != 0) {
            spdlog::error("Traffic trace_id={} type={} dir={} summary={}",
                          static_cast<unsigned long long>(event.traceId),
                          ui::common::toString(event.requestType),
                          ui::common::toString(event.direction),
                          event.summary.toStdString());
        } else {
            spdlog::error("Traffic type={} dir={} summary={}",
                          ui::common::toString(event.requestType),
                          ui::common::toString(event.direction),
                          event.summary.toStdString());
        }
        break;
    }
}

} // namespace ui::logging
