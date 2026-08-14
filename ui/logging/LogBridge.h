/**
 * @file LogBridge.h
 * @brief Strategic bridge from TrafficEvent to spdlog.
 *
 * Encapsulates TrafficEventLevel to spdlog::level mapping.
 *
 * Bridge policy (must stay aligned with docs/logging-strategy.md):
 * an event crosses the bridge iff it is Warning, Error, a Connection
 * lifecycle event, or carries a non-zero traceId.
 *
 * Governance (Task 1.5): relay() is a stateless free function and the ONLY
 * legitimate entry point into this bridge is
 * TrafficLogController::publishEvent(). Producers must emit signals wired to
 * publishEvent; calling relay() directly from any other site is forbidden.
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

/// Bridges a TrafficEvent into spdlog per the policy above. Do not call this
/// directly; route events through TrafficLogController::publishEvent().
void relay(const ui::common::TrafficEvent& event);

} // namespace ui::logging
