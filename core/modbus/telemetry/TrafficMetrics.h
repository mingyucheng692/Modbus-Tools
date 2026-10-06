/**
 * @file TrafficMetrics.h
 * @brief Telemetry data structures for Modbus traffic metrics and byte-level statistics.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QtGlobal>
#include <QString>

namespace modbus::telemetry {

/**
 * @struct TrafficStats
 * @brief Lightweight TX/RX byte and frame statistics tracker.
 */
struct TrafficStats {
    enum class Direction { Tx, Rx };

    qint64 txBytes = 0;
    qint64 rxBytes = 0;
    qint64 txFrames = 0;
    qint64 rxFrames = 0;

    void update(Direction dir, qint64 bytes) noexcept {
        if (dir == Direction::Tx) {
            txBytes += bytes;
            ++txFrames;
        } else {
            rxBytes += bytes;
            ++rxFrames;
        }
    }

    [[nodiscard]] QString formatStats() const {
        return QStringLiteral("TX: %1 | RX: %2")
            .arg(formatBytes(txBytes), formatBytes(rxBytes));
    }

    void reset() noexcept {
        txBytes = 0;
        rxBytes = 0;
        txFrames = 0;
        rxFrames = 0;
    }

    [[nodiscard]] static QString formatBytes(qint64 bytes) {
        if (bytes < 1024) {
            return QStringLiteral("%1 B").arg(bytes);
        }
        if (bytes < 1024 * 1024) {
            return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
        }
        return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
    }

    [[nodiscard]] static QString formatSize(qint64 bytes) {
        return formatBytes(bytes);
    }
};

using TrafficMetrics = TrafficStats;

} // namespace modbus::telemetry
