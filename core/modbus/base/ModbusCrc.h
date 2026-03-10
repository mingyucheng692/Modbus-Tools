#pragma once

#include <QByteArrayView>
#include <cstdint>

namespace modbus::base {

inline uint16_t calculateModbusRtuCrc(QByteArrayView data) {
    uint16_t crc = 0xFFFF;
    for (char c : data) {
        crc ^= static_cast<uint8_t>(c);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

} // namespace modbus::base
