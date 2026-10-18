// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QAbstractTableModel>
#include <deque>
#include "TelemetryRowView.h"

namespace ui::widgets::modbus {

/// Role: High-performance 5-column table model maintaining bounded ring-buffer telemetry rows.
/// Thread-safety: Thread-affine (GUI main thread event loop).
/// Ownership: Owned by ModbusTelemetryWidget via Qt parent tree or std::unique_ptr.
class TelemetryTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    static constexpr int kDefaultMaxRows = 10000;

    enum Column {
        ColTimestamp = 0,
        ColDirection = 1,
        ColLength = 2,
        ColPayload = 3,
        ColStatus = 4,
        ColumnCount = 5
    };

    explicit TelemetryTableModel(QObject* parent = nullptr);
    ~TelemetryTableModel() override = default;

    [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void appendRows(const QList<TelemetryRowView>& rows);
    void clear();

    [[nodiscard]] const TelemetryRowView* rowAt(int row) const noexcept;
    [[nodiscard]] int findRowById(uint64_t rowId) const noexcept;

    [[nodiscard]] int maxRows() const noexcept { return maxRows_; }
    void setMaxRows(int maxRows) noexcept;

    [[nodiscard]] uint64_t nextRowId() noexcept { return nextRowId_++; }

private:
    std::deque<TelemetryRowView> rows_;
    int maxRows_ = kDefaultMaxRows;
    uint64_t nextRowId_ = 1;
};

} // namespace ui::widgets::modbus

