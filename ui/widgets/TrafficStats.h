/**
 * @file TrafficStats.h
 * @brief Pure C++ struct for tracking TX/RX byte statistics.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QtGlobal>
#include <QString>

namespace ui::widgets {

struct TrafficStats {
    enum class Direction { Tx, Rx };

    qint64 txBytes = 0;
    qint64 rxBytes = 0;

    void update(Direction dir, int byteCount) {
        if (byteCount <= 0) {
            return;
        }
        if (dir == Direction::Tx) {
            txBytes += byteCount;
        } else {
            rxBytes += byteCount;
        }
    }

    QString formatStats() const {
        return QStringLiteral("TX: %1 | RX: %2")
            .arg(formatSize(txBytes), formatSize(rxBytes));
    }

    void reset() {
        txBytes = 0;
        rxBytes = 0;
    }

    static QString formatSize(qint64 bytes) {
        if (bytes < 1024) {
            return QStringLiteral("%1 B").arg(bytes);
        }
        if (bytes < 1024 * 1024) {
            return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
        }
        return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
    }
};

} // namespace ui::widgets
