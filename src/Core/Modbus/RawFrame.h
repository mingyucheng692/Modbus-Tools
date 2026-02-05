#pragma once
#include <vector>
#include <cstdint>
#include <chrono>

enum class Direction {
    Tx,
    Rx
};

struct RawFrame {
    std::chrono::system_clock::time_point timestamp;
    Direction direction;
    std::vector<uint8_t> data;
};
