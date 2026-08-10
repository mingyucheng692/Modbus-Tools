/**
 * @file LogBridge.h
 * @brief Strategic bridge from TrafficEvent to spdlog.
 *
 * Encapsulates TrafficEventLevel to spdlog::level mapping.
 * Error-level events always cross the bridge. A small whitelist of
 * lifecycle/request-summary events (for example Connection and traced
 * request events) also cross so that UI-only observability does not
 * leave gaps in the persistent log.
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
