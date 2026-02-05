#pragma once
#include <QObject>
#include <spdlog/spdlog.h>
#include "HAL/TcpChannel.h"
#include "Modbus/ModbusTcpClient.h"

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
    void disconnect();
    void sendRequest(int slaveId, int funcCode, int startAddr, int count, const QString& dataHex);

signals:
    void workerReady();
    void testResponse(const QString& msg);
    
    void connectionStateChanged(bool connected);
    void errorOccurred(const QString& msg);

private:
    TcpChannel* tcpChannel_ = nullptr;
    Modbus::ModbusTcpClient* modbusClient_ = nullptr;
};
