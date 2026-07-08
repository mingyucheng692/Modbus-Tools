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
#include <QDateTime>

namespace ui::logging {

namespace {

// Deduplication state for poll Warning events to prevent log flooding.
// Only the first Warning within a 1-second window for a given poll event
// is forwarded to spdlog; subsequent identical events are suppressed.
QString lastPollWarningSummary_;
qint64 lastPollWarningMs_ = 0;

constexpr qint64 kPollWarningDedupMs = 1000;

} // namespace

void relay(const ui::common::TrafficEvent& event)
{
    if (event.level == ui::common::TrafficEventLevel::Error) {
        spdlog::error("Traffic type={} dir={} summary={}",
            ui::common::toString(event.requestType),
            ui::common::toString(event.direction),
            event.summary.toStdString());
    } else if (event.level == ui::common::TrafficEventLevel::Warning) {
        // Deduplicate poll warnings at 1-second granularity
        if (event.isPoll) {
            const qint64 now = QDateTime::currentMSecsSinceEpoch();
            if (event.summary == lastPollWarningSummary_ && (now - lastPollWarningMs_) < kPollWarningDedupMs) {
                return;
            }
            lastPollWarningSummary_ = event.summary;
            lastPollWarningMs_ = now;
        }
        spdlog::warn("Traffic type={} dir={} summary={}",
            ui::common::toString(event.requestType),
            ui::common::toString(event.direction),
            event.summary.toStdString());
    }
}

} // namespace ui::logging
