#pragma once
#include <cstdint>
#include <bit>

namespace Utils {

inline uint16_t swap16(uint16_t val) {
    return (val << 8) | (val >> 8);
}

inline uint32_t swap32(uint32_t val) {
    return ((val << 24) & 0xff000000) |
           ((val << 8)  & 0x00ff0000) |
           ((val >> 8)  & 0x0000ff00) |
           ((val >> 24) & 0x000000ff);
}

template<typename T>
T toBigEndian(T val) {
    if constexpr (std::endian::native == std::endian::big) {
        return val;
    } else {
        if constexpr (sizeof(T) == 2) return swap16(val);
        else if constexpr (sizeof(T) == 4) return swap32(val);
        else return val; 
    }
}

template<typename T>
T fromBigEndian(T val) {
    return toBigEndian(val); // Symmetric
}

}
