#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>

class RingBuffer {
public:
    explicit RingBuffer(size_t capacity) : capacity_(capacity), buffer_(capacity) {}

    size_t size() const { return count_; }
    size_t capacity() const { return capacity_; }
    size_t available() const { return capacity_ - count_; }

    size_t write(const uint8_t* data, size_t len) {
        len = std::min(len, available());
        if (len == 0) return 0;

        size_t first_chunk = std::min(len, capacity_ - tail_);
        std::memcpy(&buffer_[tail_], data, first_chunk);
        
        if (len > first_chunk) {
            std::memcpy(&buffer_[0], data + first_chunk, len - first_chunk);
        }

        tail_ = (tail_ + len) % capacity_;
        count_ += len;
        return len;
    }
    
    size_t peek(uint8_t* data, size_t len) const {
        len = std::min(len, count_);
        if (len == 0) return 0;

        size_t first_chunk = std::min(len, capacity_ - head_);
        std::memcpy(data, &buffer_[head_], first_chunk);
        
        if (len > first_chunk) {
            std::memcpy(data + first_chunk, &buffer_[0], len - first_chunk);
        }
        return len;
    }

    void skip(size_t len) {
        len = std::min(len, count_);
        head_ = (head_ + len) % capacity_;
        count_ -= len;
    }
    
    size_t read(uint8_t* data, size_t len) {
        size_t r = peek(data, len);
        skip(r);
        return r;
    }

    void clear() {
        head_ = 0;
        tail_ = 0;
        count_ = 0;
    }

private:
    std::vector<uint8_t> buffer_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
    size_t capacity_;
};
