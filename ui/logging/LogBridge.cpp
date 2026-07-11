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
    if (event.level == ui::common::TrafficEventLevel::Error) {
        spdlog::error("Traffic type={} dir={} summary={}",
            ui::common::toString(event.requestType),
            ui::common::toString(event.direction),
            event.summary.toStdString());
    }
}

} // namespace ui::logging
