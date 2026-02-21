#pragma once

#include "IModbusClient.h"
#include "../transport/ITransport.h"
#include "../../io/IChannel.h"
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace modbus::session {

class ModbusClient : public IModbusClient {
public:
    ModbusClient(std::shared_ptr<io::IChannel> channel, 
                 std::shared_ptr<transport::ITransport> transport);
    ~ModbusClient() override;

    std::future<ModbusResponse> sendRequest(const base::Pdu& request, int slaveId = -1) override;
    void sendRaw(const QByteArray& data) override;
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    void setConfig(const base::ModbusConfig& config) override;

private:
    ModbusResponse sendRequestInternal(const base::Pdu& request, int slaveId);
    void onDataReceived(QByteArrayView data);
    void onError(const QString& error);

    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<transport::ITransport> transport_;
    base::ModbusConfig config_;

    // 同步机制：等待响应
    std::mutex mutex_;
    std::condition_variable cv_;
    QByteArray buffer_;
    bool responseReady_ = false;
    QString lastError_;
    
    // 保护 sendRequest 串行执行
    std::mutex requestMutex_;
};

} // namespace modbus::session
