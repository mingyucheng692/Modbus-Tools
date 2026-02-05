#pragma once
#include <QObject>
#include "HAL/IChannel.h"
#include "ModbusDefs.h"
#include "Core/Utils/RingBuffer.h"

namespace Modbus {

class ModbusRtuClient : public QObject {
    Q_OBJECT
public:
    explicit ModbusRtuClient(IChannel* channel, QObject* parent = nullptr);
    void sendRequest(uint8_t unitId, FunctionCode fc, uint16_t addr, uint16_t count);

signals:
    void responseReceived(uint8_t unitId, FunctionCode fc, const std::vector<uint8_t>& data);

private slots:
    void onDataReceived(const std::vector<uint8_t>& data);

private:
    uint16_t calculateCRC(const std::vector<uint8_t>& data);
    IChannel* channel_;
};

}
