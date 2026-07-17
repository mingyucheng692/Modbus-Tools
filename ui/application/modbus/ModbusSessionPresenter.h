#pragma once

#include <QObject>
#include <QPointer>
#include <memory>
#include <cstdint>
#include <functional>
#include "modbus/base/ModbusConfig.h"
#include "modbus/session/SessionTypes.h"
#include "../../../infra/io/IChannel.h"
#include "ModbusTypes.h"
#include "SessionConnectionStateMachine.h"

class QThread;
class QWidget;

namespace ui::widgets {
class ControlWidget;
class BaseConnectionWidget;
}

namespace ui::application::modbus {
class RequestSubmissionService;
class PollingController;
class TrafficLogController;
class WorkerReleaseCoordinator;
}

namespace modbus::dispatch { class ModbusWorker; }
namespace modbus::factory { class ModbusFactory; }
namespace modbus::session { class ModbusClient; }

namespace ui::application::modbus {

// SessionConnectionState is defined in SessionConnectionStateMachine.h.

/**
 * @brief Session presenter bridging the UI layer and Modbus worker thread.
 *
 * @thread Lives on the GUI thread. All public methods must be called from the
 *         GUI thread. Cross-thread communication with the ModbusWorker happens
 *         exclusively via queued signal/slot connections (no DirectConnection).
 *         Worker thread teardown is delegated to WorkerReleaseCoordinator,
 *         which enforces a bounded, non-terminating shutdown.
 *
 * @guarded_by Qt event loop — all member writes occur on the GUI thread via
 *             queued signal delivery. No explicit mutex needed as Qt's event
 *             system provides implicit serialization.
 */
class ModbusSessionPresenter : public QObject {
    Q_OBJECT

public:
    explicit ModbusSessionPresenter(SessionMode mode,
                                    ::modbus::factory::ModbusFactory* factory = nullptr,
                                    QObject* parent = nullptr);
    ~ModbusSessionPresenter() noexcept override;

    void requestConnect(const ModbusConnectionSpec& spec);
    void connectTcp(const QString& ip, int port, const ::modbus::base::ModbusConfig& config);
    void connectRtu(const io::SerialConfig& serialConfig, const ::modbus::base::ModbusConfig& modbusConfig);
    void requestDisconnect();
    void shutdown();
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
    void setConnectionWidget(ui::widgets::BaseConnectionWidget* widget);
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
    void startConnect(const ModbusConnectionSpec& spec);
    void startTcpConnect(const QString& ip, int port, const ::modbus::base::ModbusConfig& config);
    void startSerialConnect(const io::SerialConfig& serialConfig, const ::modbus::base::ModbusConfig& modbusConfig);
    void setupChannelMonitor(quint64 generation);
    void setupChannelStateHandler(quint64 generation);
    void setupWorkerSignals(quint64 generation);
    // Common tail of startTcpConnect/startSerialConnect: applies timing config to the
    // worker, wires channel/worker signal handlers for this generation, and starts
    // the IO thread + worker before requesting a connect. Deduped so the two
    // transport paths cannot drift.
    void activateStack(quint64 generation);
    void handleChannelStateTransition(io::ChannelState state, quint64 generation);
    void handleConnectFinished(bool ok, const QString& error, quint64 generation);
    void handleRequestFinished(int requestId, const ::modbus::session::ModbusResponse& response,
                               quint64 generation);
    void assertGuiThread(const char* context) const;
    void onConnectionStateChanged(SessionConnectionState state);
    void syncConnectionWidget(SessionConnectionState state);
    bool hasLiveOrPendingStack() const;
    // Hands the current live stack off to releaseCoordinator_ for bounded
    // async teardown. Performs all session-state side effects (generation
    // bump, linked/control-widget reset, linkageSourceDisconnected emission)
    // before delegating; the coordinator only owns thread lifecycle.
    void requestRelease(const QString& timeoutMessage);
    void onReleaseCompleted();
    void onReleaseTimedOut(const QString& message);
    void maybeRunDeferredAction();

    SessionMode mode_;
    ::modbus::factory::ModbusFactory* factory_ = nullptr; // optional injected factory; nullptr → create concrete ModbusFactory in initStack
    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<::modbus::session::ModbusClient> client_;
    std::shared_ptr<::modbus::dispatch::ModbusWorker> worker_;
    std::shared_ptr<QThread> channelThread_;
    std::shared_ptr<QThread> modbusWorkerThread_;
    ::modbus::base::ModbusConfig currentConfig_;
    quint64 connectionGeneration_ = 0;
    std::unique_ptr<SessionConnectionStateMachine> connectionStateMachine_;
    bool suppressDisconnectAlert_ = false;
    bool linked_ = false;
    int timeoutMs_;
    int retries_;
    int retryIntervalMs_;

    QPointer<TrafficLogController> trafficLogController_;
    QPointer<PollingController> pollingController_;
    QPointer<RequestSubmissionService> requestService_;
    ui::widgets::BaseConnectionWidget* connectionWidget_ = nullptr;
    QPointer<ui::widgets::ControlWidget> controlWidget_;
    std::unique_ptr<WorkerReleaseCoordinator> releaseCoordinator_;
    std::function<void()> deferredAction_;
};

} // namespace ui::application::modbus
