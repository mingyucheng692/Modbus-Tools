#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>

namespace Modbus {

enum class Endianness {
    ABCD, // Big Endian (Standard Modbus)
    DCBA, // Little Endian
    BADC, // Big Endian Byte Swap
    CDAB  // Little Endian Byte Swap
};

class EndianUtils {
public:
    static uint16_t toUInt16(const uint8_t* data, Endianness endian);
    static int16_t toInt16(const uint8_t* data, Endianness endian);
    static uint32_t toUInt32(const uint8_t* data, Endianness endian);
    static float toFloat(const uint8_t* data, Endianness endian);
};

}
