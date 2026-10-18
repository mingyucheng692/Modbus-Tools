// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>
#include "ui/widgets/modbus/TelemetryTableModel.h"

using namespace ui::widgets::modbus;
using namespace ui::common;

TEST(TelemetryTableModelTest, InitialStateAndHeaders) {
    TelemetryTableModel model;
    EXPECT_EQ(model.rowCount(), 0);
    EXPECT_EQ(model.columnCount(), TelemetryTableModel::ColumnCount);

    EXPECT_EQ(model.headerData(TelemetryTableModel::ColTimestamp, Qt::Horizontal).toString(), "Timestamp (ms)");
    EXPECT_EQ(model.headerData(TelemetryTableModel::ColDirection, Qt::Horizontal).toString(), "Direction");
    EXPECT_EQ(model.headerData(TelemetryTableModel::ColLength, Qt::Horizontal).toString(), "Length");
    EXPECT_EQ(model.headerData(TelemetryTableModel::ColPayload, Qt::Horizontal).toString(), "Message HEX / Summary");
    EXPECT_EQ(model.headerData(TelemetryTableModel::ColStatus, Qt::Horizontal).toString(), "Status");
}

TEST(TelemetryTableModelTest, AppendAndReadData) {
    TelemetryTableModel model;

    TelemetryRowView row1;
    row1.rowId = model.nextRowId();
    row1.event.direction = TrafficDirection::Tx;
    row1.event.payload = QByteArray("\x01\x03\x00\x00\x00\x02", 6);
    row1.statusText = "-";
    row1.statusColor = QColor(108, 117, 125);

    TelemetryRowView row2;
    row2.rowId = model.nextRowId();
    row2.event.direction = TrafficDirection::Rx;
    row2.event.payload = QByteArray("\x01\x03\x04\x00\x01\x00\x02", 7);
    row2.statusText = "Success";
    row2.statusColor = QColor(25, 135, 84);

    TelemetryRowView row3;
    row3.rowId = model.nextRowId();
    row3.event.direction = TrafficDirection::None;
    row3.event.summary = "Connected";
    row3.statusText = "Info";
    row3.statusColor = QColor(13, 110, 253);

    model.appendRows({row1, row2, row3});
    EXPECT_EQ(model.rowCount(), 3);

    // Row 0 (TX)
    QModelIndex idx0Dir = model.index(0, TelemetryTableModel::ColDirection);
    EXPECT_EQ(model.data(idx0Dir, Qt::DisplayRole).toString(), "TX");
    QModelIndex idx0Len = model.index(0, TelemetryTableModel::ColLength);
    EXPECT_EQ(model.data(idx0Len, Qt::DisplayRole).toString(), "6");
    QModelIndex idx0Pay = model.index(0, TelemetryTableModel::ColPayload);
    EXPECT_EQ(model.data(idx0Pay, Qt::DisplayRole).toString(), "01 03 00 00 00 02");

    // Row 1 (RX)
    QModelIndex idx1Dir = model.index(1, TelemetryTableModel::ColDirection);
    EXPECT_EQ(model.data(idx1Dir, Qt::DisplayRole).toString(), "RX");
    QModelIndex idx1Stat = model.index(1, TelemetryTableModel::ColStatus);
    EXPECT_EQ(model.data(idx1Stat, Qt::DisplayRole).toString(), "Success");

    // Row 2 (SYS)
    QModelIndex idx2Dir = model.index(2, TelemetryTableModel::ColDirection);
    EXPECT_EQ(model.data(idx2Dir, Qt::DisplayRole).toString(), "SYS");
    QModelIndex idx2Len = model.index(2, TelemetryTableModel::ColLength);
    EXPECT_EQ(model.data(idx2Len, Qt::DisplayRole).toString(), "-");
    QModelIndex idx2Pay = model.index(2, TelemetryTableModel::ColPayload);
    EXPECT_EQ(model.data(idx2Pay, Qt::DisplayRole).toString(), "Connected");

    // Query by pointer & ID
    const TelemetryRowView* pRow1 = model.rowAt(0);
    ASSERT_NE(pRow1, nullptr);
    EXPECT_EQ(pRow1->rowId, row1.rowId);
    EXPECT_EQ(model.findRowById(row2.rowId), 1);
    EXPECT_EQ(model.findRowById(99999), -1);
}

TEST(TelemetryTableModelTest, RingBufferEviction) {
    TelemetryTableModel model;
    model.setMaxRows(5);

    QList<TelemetryRowView> batch1;
    for (int i = 0; i < 3; ++i) {
        TelemetryRowView r;
        r.rowId = model.nextRowId();
        r.event.summary = QString("Row %1").arg(r.rowId);
        batch1.append(r);
    }
    model.appendRows(batch1);
    EXPECT_EQ(model.rowCount(), 3);

    QList<TelemetryRowView> batch2;
    for (int i = 0; i < 4; ++i) {
        TelemetryRowView r;
        r.rowId = model.nextRowId();
        r.event.summary = QString("Row %1").arg(r.rowId);
        batch2.append(r);
    }
    model.appendRows(batch2);
    // 3 + 4 = 7, max = 5 -> oldest 2 dropped (rowId 1 and 2), keeping 3, 4, 5, 6, 7
    EXPECT_EQ(model.rowCount(), 5);
    EXPECT_EQ(model.findRowById(1), -1);
    EXPECT_EQ(model.findRowById(2), -1);
    EXPECT_EQ(model.findRowById(3), 0);
    EXPECT_EQ(model.findRowById(7), 4);
}

TEST(TelemetryTableModelTest, ClearModel) {
    TelemetryTableModel model;
    TelemetryRowView r;
    r.rowId = model.nextRowId();
    model.appendRows({r});
    EXPECT_EQ(model.rowCount(), 1);

    model.clear();
    EXPECT_EQ(model.rowCount(), 0);
}

