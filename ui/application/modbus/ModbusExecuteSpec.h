// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QByteArray>
#include <cstdint>

namespace ui::application::modbus {

/// Role: Single-source-of-truth execution specification capturing standard and raw Modbus requests.
/// Thread-safety: Thread-compatible plain data structure.
/// Ownership: Value object passed by value or const reference across application and UI layers.
struct ModbusExecuteSpec {
    uint8_t slaveId = 1;
    uint8_t functionCode = 0x03;
    uint16_t startAddress = 0;
    uint16_t quantity = 10;
    QByteArray writeData;
    bool isRawMode = false;
    QByteArray rawPduOrAdu;
};

} // namespace ui::application::modbus

