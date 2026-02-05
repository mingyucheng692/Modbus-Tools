#include "TrafficLog.h"

TrafficLog& TrafficLog::instance() {
    static TrafficLog instance;
    return instance;
}

void TrafficLog::addFrame(const RawFrame& frame) {
    size_t idx;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        frames_.push_back(frame);
        idx = frames_.size() - 1;
    }
    emit frameAdded(idx);
}

const RawFrame& TrafficLog::getFrame(size_t index) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index < frames_.size()) {
        return frames_[index];
    }
    static RawFrame empty;
    return empty;
}

size_t TrafficLog::count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return frames_.size();
}

void TrafficLog::clear() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        frames_.clear();
    }
    emit cleared();
}
