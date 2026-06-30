#pragma once

#include <QByteArrayView>
#include <cstdint>

namespace modbus::base {

uint8_t calculateModbusAsciiLrc(QByteArrayView data);

} // namespace modbus::base
