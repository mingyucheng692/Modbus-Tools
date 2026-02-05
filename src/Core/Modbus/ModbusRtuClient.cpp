#include "ModbusRtuClient.h"

using namespace Modbus;

ModbusRtuClient::ModbusRtuClient(IChannel* channel, QObject* parent) : QObject(parent), channel_(channel) {
    connect(channel_, &IChannel::dataReceived, this, &ModbusRtuClient::onDataReceived);
}

void ModbusRtuClient::sendRequest(uint8_t unitId, FunctionCode fc, uint16_t addr, uint16_t count, const std::vector<uint8_t>& data) {
    std::vector<uint8_t> frame;
    frame.push_back(unitId);
    frame.push_back(static_cast<uint8_t>(fc));
    frame.push_back(addr >> 8);
    frame.push_back(addr & 0xFF);
    
    // Simplified support for Read functions
    if (fc == FunctionCode::ReadHoldingRegisters || fc == FunctionCode::ReadInputRegisters || 
        fc == FunctionCode::ReadCoils || fc == FunctionCode::ReadDiscreteInputs) {
        frame.push_back(count >> 8);
        frame.push_back(count & 0xFF);
    } else if (fc == FunctionCode::WriteSingleCoil || fc == FunctionCode::WriteSingleRegister) {
        if (data.size() >= 2) {
             frame.push_back(data[0]);
             frame.push_back(data[1]);
        } else {
             frame.push_back(count >> 8);
             frame.push_back(count & 0xFF);
        }
    } else if (fc == FunctionCode::WriteMultipleCoils || fc == FunctionCode::WriteMultipleRegisters) {
        frame.push_back(count >> 8);
        frame.push_back(count & 0xFF);
        
        uint8_t byteCount = static_cast<uint8_t>(data.size());
        frame.push_back(byteCount);
        frame.insert(frame.end(), data.begin(), data.end());
    }
    
    uint16_t crc = calculateCRC(frame);
    frame.push_back(crc & 0xFF); // Low byte first
    frame.push_back(crc >> 8);   // High byte
    
    channel_->write(frame);
}

void ModbusRtuClient::onDataReceived(const std::vector<uint8_t>& data) {
    // SerialChannel emits dataReceived after silence, so we assume it's a potential frame.
    if (data.size() < 4) return;
    
    // Check CRC
    uint16_t receivedCrc = data[data.size()-2] | (data[data.size()-1] << 8);
    
    std::vector<uint8_t> content(data.begin(), data.end() - 2);
    uint16_t calcCrc = calculateCRC(content);
    
    if (receivedCrc == calcCrc) {
        uint8_t uid = content[0];
        uint8_t fc = content[1];
        std::vector<uint8_t> pdu(content.begin() + 2, content.end());
        emit responseReceived(uid, static_cast<FunctionCode>(fc), pdu);
    }
}

uint16_t ModbusRtuClient::calculateCRC(const std::vector<uint8_t>& data) {
    uint16_t crc = 0xFFFF;
    for (uint8_t b : data) {
        crc ^= b;
        for (int i = 0; i < 8; i++) {
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
