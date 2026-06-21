#pragma once

#include <QObject>
#include <QPointer>
#include <memory>
#include <cstdint>
#include <vector>
#include "modbus/base/ModbusConfig.h"
#include "modbus/session/IModbusClient.h"
#include "../../../infra/io/IChannel.h"
#include "ModbusTypes.h"

class QThread;
class QTimer;
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

enum class SessionConnectionState {
    Disconnected,
    Connecting,
    TransportConnected,
    Connected,
    Disconnecting
};

/**
 * @brief Session presenter bridging the UI layer and Modbus worker thread.
 *
 * @thread Lives on the GUI thread. All public methods must be called from the
 *         GUI thread. Cross-thread communication with the ModbusWorker happens
 *         exclusively via queued signal/slot connections (no DirectConnection).
 *         Worker thread lifecycle managed via shared_ptr<PendingReleaseContext>.
 *
 * @guarded_by Qt event loop — all member writes occur on the GUI thread via
 *             queued signal delivery. No explicit mutex needed as Qt's event
 *             system provides implicit serialization.
 */
class ModbusSessionPresenter : public QObject {
    Q_OBJECT

public:
    explicit ModbusSessionPresenter(SessionMode mode, QObject* parent = nullptr);
    ~ModbusSessionPresenter() noexcept override;

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
    struct PendingReleaseContext;

    void initStack(const ::modbus::base::ModbusConfig& config);
    void setupChannelMonitor(quint64 generation);
    void setupChannelStateHandler(quint64 generation);
    void setupWorkerSignals(quint64 generation);
    void handleChannelStateTransition(io::ChannelState state, quint64 generation);
    void handleConnectFinished(bool ok, const QString& error, quint64 generation);
    void handleRequestFinished(int requestId, const ::modbus::session::ModbusResponse& response,
                               quint64 generation);
    void setConnectionWidgetConnecting();
    void setConnectionWidgetTransportConnected();
    void setConnectionWidgetConnected();
    void setConnectionWidgetDisconnecting();
    void setConnectionWidgetDisconnected();
    void finalizePendingRelease(const std::shared_ptr<PendingReleaseContext>& pending);

    struct PendingReleaseContext {
        std::shared_ptr<io::IChannel> channel;
        std::shared_ptr<::modbus::session::IModbusClient> client;
        std::shared_ptr<::modbus::dispatch::ModbusWorker> worker;
        std::shared_ptr<QThread> ioThread;
        std::shared_ptr<QThread> thread;
        QTimer* timeoutTimer = nullptr;
    };

    SessionMode mode_;
    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<::modbus::session::IModbusClient> client_;
    std::shared_ptr<::modbus::dispatch::ModbusWorker> worker_;
    std::shared_ptr<QThread> ioThread_;
    std::shared_ptr<QThread> workerThread_;
    ::modbus::base::ModbusConfig currentConfig_;
    quint64 connectionGeneration_ = 0;
    SessionConnectionState connectionState_ = SessionConnectionState::Disconnected;
    bool suppressDisconnectAlert_ = false;
    bool linked_ = false;
    int timeoutMs_;
    int retries_;
    int retryIntervalMs_;

    QPointer<TrafficLogController> trafficLogController_;
    QPointer<PollingController> pollingController_;
    QPointer<RequestSubmissionService> requestService_;
    QPointer<QWidget> connectionWidget_;
    QPointer<ui::widgets::ControlWidget> controlWidget_;
    std::vector<std::shared_ptr<PendingReleaseContext>> pendingReleases_;
};

} // namespace ui::application::modbus
