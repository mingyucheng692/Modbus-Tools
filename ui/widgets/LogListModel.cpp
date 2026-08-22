/**
 * @file LogListModel.cpp
 * @brief Implementation of LogListModel.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "LogListModel.h"
#include "Config.h"

namespace ui::widgets {

LogListModel::LogListModel(int maxBlockCount, QObject* parent)
    : QAbstractListModel(parent),
      maxBlockCount_(qMax(1, maxBlockCount)) {}

LogListModel::LogListModel(QObject* parent)
    : LogListModel(config::Ui::kTrafficMonitorMaxBlockCount, parent) {}

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
    const int maxRows = maxBlockCount_;
    QList<LogEntry> entriesToAppend = newEntries;
    if (entriesToAppend.size() > maxRows) {
        entriesToAppend.erase(entriesToAppend.begin(), entriesToAppend.begin() + (entriesToAppend.size() - maxRows));
    }
    const int overflow = qMax(0, entries_.size() + entriesToAppend.size() - maxRows);
    if (overflow > 0) {
        beginRemoveRows(QModelIndex(), 0, overflow - 1);
        entries_.erase(entries_.begin(), entries_.begin() + overflow);
        endRemoveRows();
    }

    const int beginRow = entries_.size();
    const int endRow = beginRow + entriesToAppend.size() - 1;
    beginInsertRows(QModelIndex(), beginRow, endRow);
    entries_.reserve(entries_.size() + entriesToAppend.size());
    for (auto&& entry : entriesToAppend) {
        entries_.append(std::move(entry));
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

void LogListModel::setMaxBlockCount(int count) {
    if (count < 1) {
        return;
    }
    maxBlockCount_ = count;
    if (entries_.size() > maxBlockCount_) {
        const int excess = entries_.size() - maxBlockCount_;
        beginRemoveRows(QModelIndex(), 0, excess - 1);
        entries_.erase(entries_.begin(), entries_.begin() + excess);
        endRemoveRows();
    }
}

} // namespace ui::widgets
