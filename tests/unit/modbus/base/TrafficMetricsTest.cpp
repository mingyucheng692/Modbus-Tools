// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "modbus/telemetry/TrafficMetrics.h"

using namespace modbus::telemetry;

TEST(TrafficMetricsTest, InitialState_Zeroed) {
    TrafficStats stats;
    EXPECT_EQ(stats.txBytes, 0);
    EXPECT_EQ(stats.rxBytes, 0);
    EXPECT_EQ(stats.txFrames, 0);
    EXPECT_EQ(stats.rxFrames, 0);
}

TEST(TrafficMetricsTest, Update_TracksBytesAndFrames) {
    TrafficStats stats;
    stats.update(TrafficStats::Direction::Tx, 120);
    stats.update(TrafficStats::Direction::Tx, 80);
    stats.update(TrafficStats::Direction::Rx, 250);

    EXPECT_EQ(stats.txBytes, 200);
    EXPECT_EQ(stats.txFrames, 2);
    EXPECT_EQ(stats.rxBytes, 250);
    EXPECT_EQ(stats.rxFrames, 1);
}

TEST(TrafficMetricsTest, FormatBytes_ScaleRepresentation) {
    EXPECT_EQ(TrafficStats::formatBytes(512), QStringLiteral("512 B"));
    EXPECT_EQ(TrafficStats::formatBytes(2048), QStringLiteral("2.0 KB"));
    EXPECT_EQ(TrafficStats::formatBytes(2 * 1024 * 1024), QStringLiteral("2.00 MB"));
}

TEST(TrafficMetricsTest, Reset_ClearsAllCounters) {
    TrafficStats stats;
    stats.update(TrafficStats::Direction::Tx, 100);
    stats.update(TrafficStats::Direction::Rx, 200);
    stats.reset();

    EXPECT_EQ(stats.txBytes, 0);
    EXPECT_EQ(stats.rxBytes, 0);
    EXPECT_EQ(stats.txFrames, 0);
    EXPECT_EQ(stats.rxFrames, 0);
}
