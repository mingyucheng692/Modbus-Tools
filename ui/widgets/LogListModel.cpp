/**
 * @file LogListModel.cpp
 * @brief Implementation of LogListModel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "LogListModel.h"
#include "AppConstants.h"

namespace ui::widgets {

LogListModel::LogListModel(QObject* parent)
    : QAbstractListModel(parent) {}

int LogListModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : entries_.size();
}

QVariant LogListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= entries_.size()) {
        return {};
    }
    const auto& entry = entries_.at(index.row());
    if (role == Qt::DisplayRole) {
        return entry.text;
    }
    if (role == Qt::ForegroundRole) {
        return entry.color;
    }
    return {};
}

void LogListModel::appendEntries(const QList<LogEntry>& newEntries) {
    if (newEntries.isEmpty()) {
        return;
    }
    const int maxRows = app::constants::Values::Ui::kTrafficMonitorMaxBlockCount;
    QList<LogEntry> entriesToAppend = newEntries;
    while (entriesToAppend.size() > maxRows) {
        entriesToAppend.removeFirst();
    }
    const int overflow = qMax(0, entries_.size() + entriesToAppend.size() - maxRows);
    if (overflow > 0) {
        beginRemoveRows(QModelIndex(), 0, overflow - 1);
        for (int i = 0; i < overflow; ++i) {
            entries_.removeFirst();
        }
        endRemoveRows();
    }

    const int beginRow = entries_.size();
    const int endRow = beginRow + entriesToAppend.size() - 1;
    beginInsertRows(QModelIndex(), beginRow, endRow);
    for (const auto& entry : entriesToAppend) {
        entries_.append(entry);
    }
    endInsertRows();
}

void LogListModel::clearAll() {
    if (entries_.isEmpty()) {
        return;
    }
    beginResetModel();
    entries_.clear();
    endResetModel();
}

void LogListModel::replaceEntries(const QList<LogEntry>& newEntries) {
    beginResetModel();
    entries_ = newEntries;
    endResetModel();
}

QString LogListModel::lineAt(int row) const {
    if (row < 0 || row >= entries_.size()) {
        return {};
    }
    return entries_.at(row).text;
}

} // namespace ui::widgets
