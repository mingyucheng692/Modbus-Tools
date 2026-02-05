#include "TrafficModel.h"
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>

TrafficModel::TrafficModel(QObject* parent) : QAbstractListModel(parent) {
    connect(&TrafficLog::instance(), &TrafficLog::frameAdded, this, &TrafficModel::onFrameAdded);
    connect(&TrafficLog::instance(), &TrafficLog::cleared, this, &TrafficModel::onCleared);
}

int TrafficModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return static_cast<int>(TrafficLog::instance().count());
}

QVariant TrafficModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return QVariant();
    
    if (role == Qt::DisplayRole) {
        const auto& frame = TrafficLog::instance().getFrame(index.row());
        std::stringstream ss;
        
        // Timestamp
        auto time = std::chrono::system_clock::to_time_t(frame.timestamp);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(frame.timestamp.time_since_epoch()) % 1000;
        
        std::tm tm_buf;
        #ifdef _WIN32
        localtime_s(&tm_buf, &time);
        #else
        localtime_r(&time, &tm_buf);
        #endif
        
        ss << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
        
        ss << (frame.direction == Direction::Tx ? "[TX] " : "[RX] ");
        
        // Simple hex formatting
        ss << std::hex << std::uppercase << std::setfill('0');
        for (uint8_t b : frame.data) {
            ss << std::setw(2) << static_cast<int>(b) << " ";
        }
        
        return QString::fromStdString(ss.str());
    }
    return QVariant();
}

void TrafficModel::onFrameAdded(size_t index) {
    beginInsertRows(QModelIndex(), static_cast<int>(index), static_cast<int>(index));
    endInsertRows();
}

void TrafficModel::onCleared() {
    beginResetModel();
    endResetModel();
}
