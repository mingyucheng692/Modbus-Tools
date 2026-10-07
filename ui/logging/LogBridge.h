// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

namespace ui::common {
struct TrafficEvent;
}

namespace ui::logging {

/// Bridges a TrafficEvent into spdlog per the policy above. Do not call this
/// directly; route events through TrafficLogController::publishEvent().
void relay(const ui::common::TrafficEvent& event);

} // namespace ui::logging
