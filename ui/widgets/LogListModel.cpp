// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include "LogListModel.h"
#include "Config.h"

namespace ui::widgets {

LogListModel::LogListModel(int maxBlockCount, QObject* parent)
    : QAbstractListModel(parent),
      maxBlockCount_(qMax(1, maxBlockCount)) {}

LogListModel::LogListModel(QObject* parent)
    : LogListModel(config::Ui::kTrafficMonitorMaxBlockCount, parent) {}

int LogListModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(entries_.size());
}

QVariant LogListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(entries_.size())) {
        return {};
    }
    const auto& entry = entries_[static_cast<size_t>(index.row())];
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
    const int overflow = qMax(0, static_cast<int>(entries_.size()) + static_cast<int>(entriesToAppend.size()) - maxRows);
    if (overflow > 0) {
        beginRemoveRows(QModelIndex(), 0, overflow - 1);
        for (int i = 0; i < overflow; ++i) {
            entries_.pop_front();
        }
        endRemoveRows();
    }

    const int beginRow = static_cast<int>(entries_.size());
    const int endRow = beginRow + static_cast<int>(entriesToAppend.size()) - 1;
    beginInsertRows(QModelIndex(), beginRow, endRow);
    for (auto&& entry : entriesToAppend) {
        entries_.push_back(std::move(entry));
    }
    endInsertRows();
}

void LogListModel::clearAll() {
    if (entries_.empty()) {
        return;
    }
    beginResetModel();
    entries_.clear();
    endResetModel();
}

void LogListModel::replaceEntries(const QList<LogEntry>& newEntries) {
    beginResetModel();
    entries_.clear();
    const qsizetype maxRows = qMax(1, maxBlockCount_);
    const qsizetype startIdx = qMax<qsizetype>(0, newEntries.size() - maxRows);
    for (qsizetype i = startIdx; i < newEntries.size(); ++i) {
        entries_.push_back(newEntries[i]);
    }
    endResetModel();
}

QString LogListModel::lineAt(int row) const {
    if (row < 0 || row >= static_cast<int>(entries_.size())) {
        return {};
    }
    return entries_[static_cast<size_t>(row)].text;
}

LogEntry LogListModel::entryAt(int row) const {
    if (row < 0 || row >= static_cast<int>(entries_.size())) {
        return {};
    }
    return entries_[static_cast<size_t>(row)];
}

void LogListModel::setMaxBlockCount(int count) {
    if (count < 1) {
        return;
    }
    maxBlockCount_ = count;
    const int currentSize = static_cast<int>(entries_.size());
    if (currentSize > maxBlockCount_) {
        const int excess = currentSize - maxBlockCount_;
        beginRemoveRows(QModelIndex(), 0, excess - 1);
        for (int i = 0; i < excess; ++i) {
            entries_.pop_front();
        }
        endRemoveRows();
    }
}

} // namespace ui::widgets
