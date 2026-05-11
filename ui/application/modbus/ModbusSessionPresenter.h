#pragma once

#include <QObject>
#include <memory>
#include <cstdint>
#include "modbus/base/ModbusConfig.h"
#include "modbus/session/IModbusClient.h"
#include "../../../core/io/IChannel.h"
#include "../../../core/io/SerialChannel.h"
#include "ModbusTypes.h"

class QThread;
class QWidget;

namespace io {
struct SerialConfig;
}

namespace ui::widgets {
class ControlWidget;
}

namespace ui::application::modbus {
class RequestSubmissionService;
class PollingController;
class TrafficLogController;
}

namespace modbus::dispatch { class ModbusWorker; }

namespace ui::application::modbus {

enum class SessionMode { Tcp, Rtu };

class ModbusSessionPresenter : public QObject {
    Q_OBJECT

public:
    explicit ModbusSessionPresenter(SessionMode mode, QObject* parent = nullptr);
    ~ModbusSessionPresenter() override;

    void connectTcp(const QString& ip, int port, const ::modbus::base::ModbusConfig& config);
    void connectRtu(const io::SerialConfig& serialConfig, const ::modbus::base::ModbusConfig& modbusConfig);
    void requestDisconnect();
    void releaseStack();

    void updateSettings(const ModbusTimingParams& params);

    bool isSessionConnected() const;
    quint64 connectionGeneration() const;
    SessionMode mode() const;

    void submitRequest(const ::modbus::base::Pdu& pdu, int slaveId, int requestId);
    void sendRaw(const QByteArray& data);

    void setTrafficLogController(TrafficLogController* controller);
    void setPollingController(PollingController* controller);
    void setRequestService(RequestSubmissionService* service);
    void setConnectionWidget(QWidget* widget);
    void setControlWidget(ui::widgets::ControlWidget* widget);

    void setLinked(bool linked);
    bool isLinked() const;

signals:
    void sessionConnected();
    void sessionDisconnected(const QString& reason);
    void connectFinished(bool ok, const QString& error);
    void requestFinished(int requestId, const ::modbus::session::ModbusResponse& response);
    void rawFrameReceived(bool isTx, const QByteArray& data);
    void stackReleased();
    void linkageSourceDisconnected();

private:
    void initStack(const ::modbus::base::ModbusConfig& config);
    void setupChannelMonitor(quint64 generation);
    void setupChannelStateHandler(quint64 generation);
    void setupWorkerSignals(quint64 generation);
    void handleTcpStateTransition(io::ChannelState state, quint64 generation);
    void handleRtuStateTransition(io::ChannelState state, quint64 generation);
    void handleConnectFinished(bool ok, const QString& error, quint64 generation);
    void handleRequestFinished(int requestId, const ::modbus::session::ModbusResponse& response,
                               quint64 generation);

    SessionMode mode_;
    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<::modbus::session::IModbusClient> client_;
    std::shared_ptr<::modbus::dispatch::ModbusWorker> worker_;
    std::shared_ptr<QThread> workerThread_;
    ::modbus::base::ModbusConfig currentConfig_;
    quint64 connectionGeneration_ = 0;
    bool sessionConnected_ = false;
    bool suppressDisconnectAlert_ = false;
    bool linked_ = false;
    int timeoutMs_;
    int retries_;
    int retryIntervalMs_;

    TrafficLogController* trafficLogController_ = nullptr;
    PollingController* pollingController_ = nullptr;
    RequestSubmissionService* requestService_ = nullptr;
    QWidget* connectionWidget_ = nullptr;
    ui::widgets::ControlWidget* controlWidget_ = nullptr;
};

} // namespace ui::application::modbus
