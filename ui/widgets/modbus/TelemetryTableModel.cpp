// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "TelemetryTableModel.h"
#include <QFontDatabase>

namespace ui::widgets::modbus {

TelemetryTableModel::TelemetryTableModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

int TelemetryTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(rows_.size());
}

int TelemetryTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return ColumnCount;
}

QVariant TelemetryTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    switch (section) {
    case ColTimestamp: return tr("Timestamp (ms)");
    case ColDirection: return tr("Direction");
    case ColLength:    return tr("Length");
    case ColPayload:   return tr("Message HEX / Summary");
    case ColStatus:    return tr("Status");
    default:           return QVariant();
    }
}

QVariant TelemetryTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(rows_.size())) {
        return QVariant();
    }

    const auto& row = rows_[static_cast<size_t>(index.row())];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColTimestamp:
            return row.event.timestamp.toString(QStringLiteral("hh:mm:ss.zzz"));
        case ColDirection:
            if (row.event.direction == ::ui::common::TrafficDirection::Tx) return QStringLiteral("TX");
            if (row.event.direction == ::ui::common::TrafficDirection::Rx) return QStringLiteral("RX");
            return QStringLiteral("SYS");
        case ColLength:
            if (row.event.direction == ::ui::common::TrafficDirection::None) return QStringLiteral("-");
            return QString::number(row.event.payload.size());
        case ColPayload:
            if (row.event.direction == ::ui::common::TrafficDirection::None) {
                return row.event.summary;
            }
            if (!row.event.payload.isEmpty()) {
                return QString::fromLatin1(row.event.payload.toHex(' ').toUpper());
            }
            return row.event.summary;
        case ColStatus:
            return row.statusText;
        default:
            return QVariant();
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (index.column() == ColPayload) {
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
        return static_cast<int>(Qt::AlignCenter);
    }

    if (role == Qt::ForegroundRole) {
        if (index.column() == ColStatus) {
            return row.statusColor;
        }
        if (index.column() == ColDirection) {
            if (row.event.direction == ::ui::common::TrafficDirection::Tx) return QColor(13, 110, 253);
            if (row.event.direction == ::ui::common::TrafficDirection::Rx) return QColor(25, 135, 84);
            return QColor(108, 117, 125);
        }
    }

    if (role == Qt::UserRole) {
        return static_cast<qulonglong>(row.rowId);
    }

    return QVariant();
}

void TelemetryTableModel::appendRows(const QList<TelemetryRowView>& rows) {
    if (rows.isEmpty()) {
        return;
    }

    const int toAdd = rows.size();
    if (toAdd >= maxRows_) {
        beginResetModel();
        rows_.clear();
        const int startIndex = toAdd - maxRows_;
        for (int i = startIndex; i < toAdd; ++i) {
            rows_.push_back(rows[i]);
        }
        endResetModel();
        return;
    }

    const int currentCount = static_cast<int>(rows_.size());
    if (currentCount + toAdd > maxRows_) {
        const int dropCount = (currentCount + toAdd) - maxRows_;
        beginRemoveRows(QModelIndex(), 0, dropCount - 1);
        rows_.erase(rows_.begin(), rows_.begin() + dropCount);
        endRemoveRows();
    }

    const int insertStart = static_cast<int>(rows_.size());
    beginInsertRows(QModelIndex(), insertStart, insertStart + toAdd - 1);
    for (const auto& r : rows) {
        rows_.push_back(r);
    }
    endInsertRows();
}

void TelemetryTableModel::clear() {
    beginResetModel();
    rows_.clear();
    endResetModel();
}

const TelemetryRowView* TelemetryTableModel::rowAt(int row) const noexcept {
    if (row < 0 || row >= static_cast<int>(rows_.size())) {
        return nullptr;
    }
    return &rows_[static_cast<size_t>(row)];
}

int TelemetryTableModel::findRowById(uint64_t rowId) const noexcept {
    for (size_t i = 0; i < rows_.size(); ++i) {
        if (rows_[i].rowId == rowId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void TelemetryTableModel::setMaxRows(int maxRows) noexcept {
    if (maxRows < 1) {
        maxRows = 1;
    }
    maxRows_ = maxRows;
    if (static_cast<int>(rows_.size()) > maxRows_) {
        const int dropCount = static_cast<int>(rows_.size()) - maxRows_;
        beginRemoveRows(QModelIndex(), 0, dropCount - 1);
        rows_.erase(rows_.begin(), rows_.begin() + dropCount);
        endRemoveRows();
    }
}

} // namespace ui::widgets::modbus

