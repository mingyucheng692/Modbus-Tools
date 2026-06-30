#include "ModbusLrc.h"

namespace modbus::base {

uint8_t calculateModbusAsciiLrc(QByteArrayView data)
{
    uint8_t sum = 0;
    for (const char byte : data) {
        sum = static_cast<uint8_t>(sum + static_cast<uint8_t>(byte));
    }
    return static_cast<uint8_t>(-sum);
}

} // namespace modbus::base
