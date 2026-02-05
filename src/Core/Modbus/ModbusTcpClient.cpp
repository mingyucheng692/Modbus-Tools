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

void ModbusTcpClient::sendRequest(uint8_t unitId, FunctionCode fc, uint16_t addr, uint16_t count, const std::vector<uint8_t>& data) {
    uint16_t tid = nextTransactionId_++;
    
    // Build PDU
    std::vector<uint8_t> pdu;
    pdu.push_back(static_cast<uint8_t>(fc));
    
    // Address
    pdu.push_back(addr >> 8);
    pdu.push_back(addr & 0xFF);
    
    if (fc == FunctionCode::ReadHoldingRegisters || fc == FunctionCode::ReadInputRegisters || 
        fc == FunctionCode::ReadCoils || fc == FunctionCode::ReadDiscreteInputs) {
        // Read Request: Addr + Count
        pdu.push_back(count >> 8);
        pdu.push_back(count & 0xFF);
    } else if (fc == FunctionCode::WriteSingleCoil || fc == FunctionCode::WriteSingleRegister) {
        // Write Single: Addr + Value (2 bytes)
        // For WriteSingle, 'count' parameter is used as 'value' usually, 
        // OR we can use the first 2 bytes of 'data'. 
        // Let's assume 'data' contains the 2 bytes value if provided, otherwise 'count' is used as value.
        // But to be consistent with common APIs, WriteSingle takes "Address" and "Value".
        // Here we reuse 'count' as 'value' for single write if data is empty.
        
        if (data.size() >= 2) {
             pdu.push_back(data[0]);
             pdu.push_back(data[1]);
        } else {
             pdu.push_back(count >> 8);
             pdu.push_back(count & 0xFF);
        }
    } else if (fc == FunctionCode::WriteMultipleCoils || fc == FunctionCode::WriteMultipleRegisters) {
        // Write Multiple: Addr + Count + ByteCount + Data
        pdu.push_back(count >> 8);
        pdu.push_back(count & 0xFF);
        
        uint8_t byteCount = static_cast<uint8_t>(data.size());
        pdu.push_back(byteCount);
        pdu.insert(pdu.end(), data.begin(), data.end());
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
    
    pendingTransactions_[tid] = {std::chrono::steady_clock::now()};
    
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
        
        int rtt = -1;
        auto it = pendingTransactions_.find(tid);
        if (it != pendingTransactions_.end()) {
            auto now = std::chrono::steady_clock::now();
            rtt = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.startTime).count();
            pendingTransactions_.erase(it);
        }
        
        emit responseReceived(tid, uid, static_cast<FunctionCode>(fcByte), pduData, rtt);
    }
}
