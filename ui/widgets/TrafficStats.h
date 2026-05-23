/**
 * @file TrafficStats.h
 * @brief Pure C++ struct for tracking TX/RX byte and packet statistics with EMA rate calculation.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QtGlobal>
#include <QString>
#include <cmath>

namespace ui::widgets {

struct TrafficStats {
    enum Direction { Tx, Rx };

    qint64 txBytes = 0;
    qint64 rxBytes = 0;
    qint64 txPackets = 0;
    qint64 rxPackets = 0;
    double txRate = 0.0;
    double rxRate = 0.0;

    void update(Direction dir, int byteCount) {
        if (byteCount <= 0) {
            return;
        }
        if (dir == Tx) {
            txBytes += byteCount;
            ++txPackets;
        } else {
            rxBytes += byteCount;
            ++rxPackets;
        }
    }

    void updateRate(int sampleIntervalMs) {
        if (sampleIntervalMs <= 0) {
            return;
        }
        const double intervalSec = sampleIntervalMs / 1000.0;
        const double newTxRate = (txBytes - lastTxBytes_) / 1024.0 / intervalSec;
        const double newRxRate = (rxBytes - lastRxBytes_) / 1024.0 / intervalSec;
        txRate = 0.7 * qMax(0.0, newTxRate) + 0.3 * txRate;
        rxRate = 0.7 * qMax(0.0, newRxRate) + 0.3 * rxRate;
        lastTxBytes_ = txBytes;
        lastRxBytes_ = rxBytes;
    }

    QString formatStats() const {
        return QStringLiteral("TX: %1 (%2 pkts) | RX: %3 (%4 pkts) | Rate: %5 / %6")
            .arg(formatSize(txBytes), QString::number(txPackets),
                 formatSize(rxBytes), QString::number(rxPackets),
                 formatRate(txRate), formatRate(rxRate));
    }

    void reset() {
        txBytes = 0;
        rxBytes = 0;
        txPackets = 0;
        rxPackets = 0;
        txRate = 0.0;
        rxRate = 0.0;
        lastTxBytes_ = 0;
        lastRxBytes_ = 0;
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

    static QString formatRate(double kbps) {
        if (kbps < 1024.0) {
            return QStringLiteral("%1 KB/s").arg(kbps, 0, 'f', 1);
        }
        return QStringLiteral("%1 MB/s").arg(kbps / 1024.0, 0, 'f', 2);
    }

private:
    qint64 lastTxBytes_ = 0;
    qint64 lastRxBytes_ = 0;
};

} // namespace ui::widgets