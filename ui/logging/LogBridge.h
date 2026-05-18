/**
 * @file LogBridge.h
 * @brief Strategic bridge from TrafficEvent to spdlog.
 *
 * Encapsulates TrafficEventLevel to spdlog::level mapping.
 * Only Error-level poll escalation events cross the bridge;
 * Info and Warning events remain in TrafficMonitor.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

namespace ui::common {
struct TrafficEvent;
}

namespace ui::logging {

void relay(const ui::common::TrafficEvent& event);

} // namespace ui::logging