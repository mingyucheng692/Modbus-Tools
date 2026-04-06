#pragma once

#include "AppConstants.h"
#include "IModbusClient.h"
#include "../transport/ITransport.h"
#include "../../io/IChannel.h"
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <chrono>

namespace modbus::session {

class ModbusClient : public IModbusClient {
public:
    enum class RequestState {
        Idle,
        Sending,
        Waiting,
        Completed,
        Failed,
        Aborted
    };

    ModbusClient(std::shared_ptr<io::IChannel> channel, 
                 std::shared_ptr<transport::ITransport> transport);
    ~ModbusClient() override;

    ModbusResponse sendRequest(const base::Pdu& request, int slaveId = -1) override;
    void sendRaw(const QByteArray& data) override;
    bool connect() override;
    void disconnect() override;
    bool isConnected() const override;
    void abort() override;
    void setConfig(const base::ModbusConfig& config) override;
    RequestState requestState() const;

private:
    struct PendingRequest {
        int requestId = 0;
        int slaveId = app::constants::Constants::Modbus::kDefaultSlaveId;
        int timeoutMs = app::constants::Constants::Modbus::kDefaultTimeoutMs;
        int retries = 0;
        base::FunctionCode functionCode = base::FunctionCode::ReadHoldingRegisters;
        std::chrono::steady_clock::time_point enqueueAt{};
    };

    ModbusResponse sendRequestInternal(const base::Pdu& request, int slaveId);
    void onDataReceived(QByteArrayView data);
    void onError(const QString& error);
    void transitionTo(RequestState newState, const char* reason);
    static const char* toString(RequestState state);
    bool isRtuBroadcastRequest(int slaveId, base::FunctionCode functionCode) const;
    bool shouldWaitForResponse(int slaveId, base::FunctionCode functionCode) const;
    void waitForRtuInterFrameDelay();
    bool waitForAbortableDelay(std::chrono::steady_clock::duration delay);
    void updateRtuSendWindow(qsizetype frameBytes);
    QString validateRequest(const base::Pdu& request, int slaveId) const;
    bool isRtuFrameReadyToParseLocked(std::chrono::steady_clock::time_point now) const;
    std::chrono::steady_clock::time_point nextRtuFrameBoundaryLocked() const;
    int enqueuePendingRequest(const base::Pdu& request, int slaveId);
    void finishPendingRequest(int requestId, bool success, const QString& error);
    void clearRuntimeState(bool clearPendingQueue);

    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<transport::ITransport> transport_;
    base::ModbusConfig config_;

    // 同步机制：等待响应
    std::mutex mutex_;
    std::condition_variable cv_;
    QByteArray buffer_;
    bool responseReady_ = false;
    QString lastError_;
    
    // 保护 sendRequest 串行执行，避免请求路径发生重入
    std::recursive_mutex requestMutex_;
    std::atomic<bool> aborted_ {false};
    std::atomic<RequestState> requestState_ {RequestState::Idle};
    std::mutex pendingMutex_;
    std::chrono::steady_clock::time_point lastRtuByteReceivedAt_ {};
    std::chrono::steady_clock::time_point nextRtuSendAllowedAt_ {};
    int nextRequestId_ = 1;
    std::deque<PendingRequest> pendingRequests_;
};

} // namespace modbus::session
