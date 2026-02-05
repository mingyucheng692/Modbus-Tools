#pragma once
#include <QObject>
#include <functional>
#include <map>
#include "Core/Utils/RingBuffer.h"
#include "HAL/IChannel.h"
#include "ModbusDefs.h"

#include <chrono>

namespace Modbus {

class ModbusTcpClient : public QObject {
    Q_OBJECT
public:
    explicit ModbusTcpClient(IChannel* channel, QObject* parent = nullptr);

    void sendRequest(uint8_t unitId, FunctionCode fc, uint16_t addr, uint16_t count, const std::vector<uint8_t>& data = {});

signals:
    // responseTimeMs, startAddress, count
    void responseReceived(uint16_t transactionId, uint8_t unitId, FunctionCode fc, const std::vector<uint8_t>& data, int responseTimeMs, uint16_t startAddr);
    void timeout(uint16_t transactionId);
    void error(const QString& msg);

private slots:
    void onChannelDataReceived(const std::vector<uint8_t>& data);

private:
    void processBuffer();
    
    IChannel* channel_;
    RingBuffer buffer_{4096};
    uint16_t nextTransactionId_ = 0;
    
    struct Transaction {
        std::chrono::steady_clock::time_point startTime;
        uint16_t startAddr;
        uint16_t count;
    };
    std::map<uint16_t, Transaction> pendingTransactions_;
};

}
