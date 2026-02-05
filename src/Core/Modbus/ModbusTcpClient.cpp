#include "ModbusTcpClient.h"
#include "Core/Utils/EndianUtils.h"
#include <spdlog/spdlog.h>

using namespace Modbus;
using namespace Utils;

ModbusTcpClient::ModbusTcpClient(IChannel* channel, QObject* parent) 
    : QObject(parent), channel_(channel) 
{
    connect(channel_, &IChannel::dataReceived, this, &ModbusTcpClient::onChannelDataReceived);
}

void ModbusTcpClient::sendRequest(uint8_t unitId, FunctionCode fc, uint16_t addr, uint16_t count) {
    uint16_t tid = nextTransactionId_++;
    
    // Build PDU
    std::vector<uint8_t> pdu;
    pdu.push_back(static_cast<uint8_t>(fc));
    
    // Address & Count
    pdu.push_back(addr >> 8);
    pdu.push_back(addr & 0xFF);
    
    if (fc == FunctionCode::ReadHoldingRegisters || fc == FunctionCode::ReadInputRegisters || 
        fc == FunctionCode::ReadCoils || fc == FunctionCode::ReadDiscreteInputs) {
        pdu.push_back(count >> 8);
        pdu.push_back(count & 0xFF);
    } 

    // Build ADU (MBAP + PDU)
    std::vector<uint8_t> adu;
    
    // MBAP
    uint16_t len = static_cast<uint16_t>(pdu.size() + 1); // UnitID + PDU
    
    adu.push_back(tid >> 8);
    adu.push_back(tid & 0xFF);
    adu.push_back(0); // Protocol ID High
    adu.push_back(0); // Protocol ID Low
    adu.push_back(len >> 8);
    adu.push_back(len & 0xFF);
    adu.push_back(unitId);
    
    adu.insert(adu.end(), pdu.begin(), pdu.end());
    
    channel_->write(adu);
}

void ModbusTcpClient::onChannelDataReceived(const std::vector<uint8_t>& data) {
    buffer_.write(data.data(), data.size());
    processBuffer();
}

void ModbusTcpClient::processBuffer() {
    while (buffer_.size() >= 7) { // Min MBAP size
        // Peek MBAP
        uint8_t header[7];
        buffer_.peek(header, 7);
        
        uint16_t len = (header[4] << 8) | header[5];
        size_t totalFrameSize = 6 + len;
        
        if (buffer_.size() < totalFrameSize) {
            // Wait for more data
            return; 
        }
        
        // Full frame available
        std::vector<uint8_t> frame(totalFrameSize);
        buffer_.read(frame.data(), totalFrameSize);
        
        // Parse
        uint16_t tid = (frame[0] << 8) | frame[1];
        uint8_t uid = frame[6];
        uint8_t fcByte = frame[7];
        
        std::vector<uint8_t> pduData;
        if (frame.size() > 8) {
             pduData.assign(frame.begin() + 8, frame.end());
        }
        
        emit responseReceived(tid, uid, static_cast<FunctionCode>(fcByte), pduData);
    }
}
