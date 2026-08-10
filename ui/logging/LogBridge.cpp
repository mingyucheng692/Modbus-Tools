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

namespace {

bool shouldRelay(const ui::common::TrafficEvent& event)
{
    if (event.level == ui::common::TrafficEventLevel::Warning
        || event.level == ui::common::TrafficEventLevel::Error) {
        return true;
    }

    if (event.requestType == ui::common::TrafficRequestType::Connection) {
        return true;
    }

    return event.traceId != 0;
}

} // namespace

void relay(const ui::common::TrafficEvent& event)
{
    if (!shouldRelay(event)) {
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
        } else {
            spdlog::info("Traffic type={} dir={} summary={}",
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
        } else {
            spdlog::warn("Traffic type={} dir={} summary={}",
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
