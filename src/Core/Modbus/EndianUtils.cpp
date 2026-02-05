#include "EndianUtils.h"
#include <cstring>

namespace Modbus {

uint16_t EndianUtils::toUInt16(const uint8_t* data, Endianness endian) {
    uint8_t b0 = data[0];
    uint8_t b1 = data[1];
    
    switch (endian) {
        case Endianness::ABCD: // Big Endian: High Low
            return (b0 << 8) | b1;
        case Endianness::DCBA: // Little Endian: Low High
            return (b1 << 8) | b0;
        case Endianness::BADC: // Swap bytes: Low High (Same as Little Endian for 16bit)
            return (b1 << 8) | b0; 
        case Endianness::CDAB: // Swap bytes: High Low (Same as Big Endian for 16bit)
            return (b0 << 8) | b1;
    }
    return 0;
}

int16_t EndianUtils::toInt16(const uint8_t* data, Endianness endian) {
    uint16_t val = toUInt16(data, endian);
    return static_cast<int16_t>(val);
}

// For 32-bit, input is 4 bytes: B0 B1 B2 B3
// ABCD: B0 B1 B2 B3 (Big Endian)
// DCBA: B3 B2 B1 B0 (Little Endian)
// BADC: B1 B0 B3 B2 (Big Endian Byte Swap)
// CDAB: B2 B3 B0 B1 (Little Endian Byte Swap)

uint32_t EndianUtils::toUInt32(const uint8_t* data, Endianness endian) {
    uint8_t b0 = data[0];
    uint8_t b1 = data[1];
    uint8_t b2 = data[2];
    uint8_t b3 = data[3];
    
    switch (endian) {
        case Endianness::ABCD: 
            return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
        case Endianness::DCBA: 
            return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
        case Endianness::BADC: 
            return (b1 << 24) | (b0 << 16) | (b3 << 8) | b2;
        case Endianness::CDAB: 
            return (b2 << 24) | (b3 << 16) | (b0 << 8) | b1;
    }
    return 0;
}

float EndianUtils::toFloat(const uint8_t* data, Endianness endian) {
    uint32_t val = toUInt32(data, endian);
    float f;
    std::memcpy(&f, &val, sizeof(f));
    return f;
}

}
