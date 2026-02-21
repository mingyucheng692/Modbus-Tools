#pragma once

#include <future>
#include <memory>
#include <QString>
#include "../base/ModbusFrame.h"
#include "../base/ModbusConfig.h"

namespace modbus::session {

// 响应结果类型：包含 Pdu 或错误信息
struct ModbusResponse {
    base::Pdu pdu;
    bool isSuccess = false;
    QString error;

    static ModbusResponse Success(base::Pdu pdu) {
        return {pdu, true, QString()};
    }
    
    static ModbusResponse Error(QString error) {
        return {base::Pdu(), false, error};
    }
};

class IModbusClient {
public:
    virtual ~IModbusClient() = default;

    // 发送请求并等待响应（同步或异步通过 Future）
    // 为了支持 Qt UI 线程不阻塞，建议内部实现为异步，这里返回 std::future
    // slaveId: 如果为 -1，则使用 setConfig 设置的默认 slaveId
    virtual std::future<ModbusResponse> sendRequest(const base::Pdu& request, int slaveId = -1) = 0;

    // 连接/断开
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    
    // 更新配置
    virtual void setConfig(const base::ModbusConfig& config) = 0;
};

} // namespace modbus::session
