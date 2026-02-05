#include "LogModel.h"

LogModel::LogModel(QObject* parent) : QAbstractListModel(parent) {}

int LogModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(logs_.size());
}

QVariant LogModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= logs_.size()) return QVariant();

    const auto& entry = logs_[index.row()];

    if (role == Qt::DisplayRole) {
        return entry.message;
    } else if (role == Qt::ForegroundRole) {
        switch (entry.level) {
            case 2: return QColor(Qt::white); // Info
            case 3: return QColor(Qt::yellow); // Warn
            case 4: 
            case 5: return QColor(Qt::red); // Error
            default: return QColor(Qt::gray);
        }
    }
    return QVariant();
}

void LogModel::addLog(const QString& msg, int level) {
    beginInsertRows(QModelIndex(), static_cast<int>(logs_.size()), static_cast<int>(logs_.size()));
    logs_.push_back({msg, level});
    endInsertRows();
}

void LogModel::clear() {
    beginResetModel();
    logs_.clear();
    endResetModel();
}
