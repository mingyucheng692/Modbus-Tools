#pragma once

#include <QObject>
#include <QPointer>
#include <memory>
#include <cstdint>
#include <functional>
#include "modbus/base/ModbusConfig.h"
#include "modbus/session/SessionTypes.h"
#include "modbus/session/ConnectionStateMachine.h"
#include "../../../infra/io/IChannel.h"
#include "ModbusTypes.h"

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
namespace modbus::session { class ModbusClient; }

namespace ui::application::modbus {

/**
 * @brief UI-layer connection state, derived from the authoritative core
 *        ConnectionStateMachine plus channel state and session health.
 *
 * Formerly owned by the deleted SessionConnectionStateMachine QObject FSM.
 * Transition validation now lives in the presenter's
 * private guard function transitionConnectionStateTo().
 */
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

    void submitRequest(const ::modbus::base::Pdu& pdu, int slaveId, int requestId,
                       TraceId traceId);
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
    void sessionTransientDisconnect(const QString& reason);
    void connectFinished(bool ok, const QString& error);
    void requestFinished(int requestId, const ::modbus::session::ModbusResponse& response);
    void rawFrameReceived(bool isTx, const QByteArray& data);
    void stackReleased();
    void stackReleaseTimedOut(const QString& message);
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

    /// Derives the UI connection state from the authoritative core
    /// ConnectionStateMachine::State, the current channel state, the
    /// session health (whether the Modbus device is actually responding),
    /// and the active session mode (TCP vs Serial).
    /// This is the single source of truth for UI state.
    static SessionConnectionState deriveUiState(
        ::modbus::session::ConnectionStateMachine::State coreState,
        io::ChannelState channelState,
        ::modbus::session::SessionHealth health,
        SessionMode mode = SessionMode::Tcp);

    /// Queries client_->connectionState() (thread-safe via std::atomic) and
    /// transitions the UI FSM to the derived state. If the derived state
    /// conflicts with the existing UI FSM state, the core state wins.
    void syncStateFromCore();

    void onConnectionStateChanged(SessionConnectionState state);
    void syncConnectionWidget(SessionConnectionState state);

    /// Validated UI connection-state transition (guard function).
    /// Rejects illegal transitions (returns false, state unchanged, logs an
    /// error) and applies the state-entry side effects (alert-suppression
    /// flags, widget sync) inline on success. No-op re-entry succeeds.
    [[nodiscard]] bool transitionConnectionStateTo(SessionConnectionState target);

    /// Forces the UI state without transition-rule validation. Only for use
    /// when the core authoritative state conflicts with the UI transition
    /// rules (core wins; safe fallback is Disconnected).
    void forceConnectionStateTo(SessionConnectionState target);

    /// Human-readable state name for logging.
    [[nodiscard]] static const char* connectionStateName(SessionConnectionState s);

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
    std::shared_ptr<io::IChannel> channel_;
    std::shared_ptr<::modbus::session::ModbusClient> client_;
    std::shared_ptr<::modbus::dispatch::ModbusWorker> worker_;
    std::shared_ptr<QThread> channelThread_;
    std::shared_ptr<QThread> modbusWorkerThread_;
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
    // Raw pointer: RequestSubmissionService is a plain C++ class owned by
    // ModbusPagePresenter (std::unique_ptr). It outlives this presenter in
    // every teardown path (services are deleted before the session
    // presenter's queued handlers can run again), so no QPointer guard is
    // possible or needed.
    RequestSubmissionService* requestService_ = nullptr;
    QPointer<ui::widgets::BaseConnectionWidget> connectionWidget_;
    QPointer<ui::widgets::ControlWidget> controlWidget_;
    std::unique_ptr<WorkerReleaseCoordinator> releaseCoordinator_;
    std::function<void()> deferredAction_;
};

} // namespace ui::application::modbus
