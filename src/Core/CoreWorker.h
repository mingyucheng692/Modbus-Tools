#pragma once
#include <QObject>
#include <spdlog/spdlog.h>
#include "HAL/TcpChannel.h"
#include "HAL/SerialChannel.h"
#include "Modbus/ModbusTcpClient.h"
#include "Modbus/ModbusRtuClient.h"

class CoreWorker : public QObject {
    Q_OBJECT
public:
    explicit CoreWorker(QObject* parent = nullptr);
    ~CoreWorker();

public slots:
    void init();
    void cleanup();
    void testWorker();
    
    void connectTcp(const QString& ip, int port);
    void connectRtu(const QString& portName, int baudRate, int dataBits, int stopBits, int parity);
    void connectSerial(const QString& portName, int baudRate, int dataBits, int stopBits, int parity);
    void disconnect();
    void sendRequest(int slaveId, int funcCode, int startAddr, int count, const QString& dataHex);
    void setPolling(bool enabled, int intervalMs);
    void setSimulation(int dropRate, int minDelay, int maxDelay);
    
    // Generic Send
    void sendRaw(const std::vector<uint8_t>& data);

    Modbus::ModbusTcpClient* modbusClient() const { return modbusClient_; }

signals:
    void workerReady();
    void testResponse(const QString& msg);
    
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString& msg);

private slots:
    void onPollTimeout();

private:
    TcpChannel* tcpChannel_ = nullptr;
    Modbus::ModbusTcpClient* modbusClient_ = nullptr;

    SerialChannel* serialChannel_ = nullptr;
    Modbus::ModbusRtuClient* rtuClient_ = nullptr;
    IChannel* activeChannel_ = nullptr;
    
    QTimer* pollTimer_ = nullptr;
    struct PollRequest {
        int slaveId;
        int funcCode;
        int startAddr;
        int count;
        QString dataHex;
    } lastRequest_;
};
