#pragma once

#include <cstdint>

namespace modbus::base {

enum class FunctionCode : uint8_t {
    ReadCoils = 0x01,
    ReadDiscreteInputs = 0x02,
    ReadHoldingRegisters = 0x03,
    ReadInputRegisters = 0x04,
    WriteSingleCoil = 0x05,
    WriteSingleRegister = 0x06,
    WriteMultipleCoils = 0x0F,
    WriteMultipleRegisters = 0x10,
    
    // 异常响应的功能码通常是 (FunctionCode | 0x80)
    ErrorMask = 0x80
};

enum class ExceptionCode : uint8_t {
    None = 0x00,
    IllegalFunction = 0x01,
    IllegalDataAddress = 0x02,
    IllegalDataValue = 0x03,
    ServerDeviceFailure = 0x04,
    Acknowledge = 0x05,
    ServerDeviceBusy = 0x06,
    NegativeAcknowledge = 0x07,
    MemoryParityError = 0x08,
    GatewayPathUnavailable = 0x0A,
    GatewayTargetDeviceFailed = 0x0B
};

struct ModbusAddress {
    enum class Area {
        Coil,
        DiscreteInput,
        HoldingRegister,
        InputRegister
    };

    Area area;
    uint16_t startAddress;
    uint16_t count;

    bool isValid() const {
        // Modbus 协议通常限制单次读写数量，这里做基础校验
        // 具体限制可能依赖于设备或功能码
        return count > 0 && count <= 2000; 
    }
};

} // namespace modbus::base
