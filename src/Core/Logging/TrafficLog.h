#pragma once
#include <vector>
#include <mutex>
#include "Core/Modbus/RawFrame.h"
#include <QObject>

class TrafficLog : public QObject {
    Q_OBJECT
public:
    static TrafficLog& instance();
    
    void addFrame(const RawFrame& frame);
    const RawFrame& getFrame(size_t index) const;
    size_t count() const;
    void clear();

signals:
    void frameAdded(size_t index);
    void cleared();

private:
    TrafficLog() = default;
    std::vector<RawFrame> frames_;
    mutable std::mutex mutex_;
};
