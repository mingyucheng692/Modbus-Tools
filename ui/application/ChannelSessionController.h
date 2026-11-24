// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "infra/io/IChannel.h"
#include "infra/io/SerialConfig.h"
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <memory>
#include <functional>
#include <array>

class QThread;
class QTimer;

namespace io {
class ChannelOperationWorker;
class ServerChannelWorker;
}

namespace ui::application {

enum class ChannelRole {
    None = 0,
    TcpClient,
    TcpServer,
    Udp,
    Serial
};

struct SessionUiSnapshot {
    io::ChannelState state = io::ChannelState::Closed;
    QString stateText;                      // State description (e.g. Disconnected, Connecting, Connected, Listening, Bound)
    bool stateChanged = false;              // True if this snapshot was triggered by a ChannelState transition
    bool protocolSelectorEnabled = true;    // state == Closed || state == Error
    bool serialControlLinesEnabled = false; // state == Open && role == Serial
    bool sendControlsEnabled = false;       // state == Open && (role != TcpServer || serverHasTargets)
    QString sendDisabledTooltip;            // Tooltip when sending is disabled
};

struct Transition {
    io::ChannelState from;
    io::ChannelState to;
};

// Compile-time table of legitimate state transitions.
inline constexpr std::array<Transition, 11> kValidTransitions = {{
    {io::ChannelState::Closed, io::ChannelState::Opening},
    {io::ChannelState::Error, io::ChannelState::Opening},
    {io::ChannelState::Opening, io::ChannelState::Open},
    {io::ChannelState::Opening, io::ChannelState::Error},
    {io::ChannelState::Opening, io::ChannelState::Closing},
    {io::ChannelState::Opening, io::ChannelState::Closed},
    {io::ChannelState::Open, io::ChannelState::Closing},
    {io::ChannelState::Open, io::ChannelState::Error},
    {io::ChannelState::Open, io::ChannelState::Closed},
    {io::ChannelState::Closing, io::ChannelState::Closed},
    {io::ChannelState::Closing, io::ChannelState::Error}
}};

// Role: Authoritative edge-driven session finite state machine managing channel workers and declaratively computing SessionUiSnapshot.
// Thread-safety: Thread-compatible (bound to GUI/main thread; delegates to workers via Qt::QueuedConnection).
// Ownership: Owned by views or parent components via QObject tree; lazily manages worker threads.
class ChannelSessionController : public QObject {
    Q_OBJECT

public:
    explicit ChannelSessionController(QObject* parent = nullptr);
    ~ChannelSessionController() noexcept override;

    // Session opening actions
    void openTcp(const QString& host, int port);
    void openUdp(const QString& localIp, int localPort, const QString& remoteIp, int remotePort);
    void openSerial(const io::SerialConfig& config);
    void openTcpServer(const QString& listenIp, int port, int maxClients = 10);
    void closeChannel();

    // Data transmission
    void write(const QByteArray& data);
    void writeToClient(int clientId, const QByteArray& data);
    void writeToClients(const QList<int>& clientIds, const QByteArray& data);

    // Target state for TCP Server mode
    void setServerTargetState(bool hasTargetClients, bool broadcastMode);
    void setRole(ChannelRole role);

    // Auto-send management
    void setAutoSend(bool enabled, int intervalMs,
                     std::function<QByteArray()> dataSupplier = nullptr,
                     std::function<void(const QByteArray&)> sendAction = nullptr);
    void setAutoSendInterval(int intervalMs);
    [[nodiscard]] bool isAutoSendActive() const;

    // Hardware control lines (Serial)
    void setDtr(bool set);
    void setRts(bool set);

    // TCP Server client controls
    void disconnectClients(const QList<int>& clientIds);
    void disconnectAllClients();

    // State & Snapshot inspection
    [[nodiscard]] const SessionUiSnapshot& currentSnapshot() const noexcept { return currentSnapshot_; }
    [[nodiscard]] io::ChannelState state() const noexcept { return state_; }
    [[nodiscard]] ChannelRole role() const noexcept { return activeRole_; }
    [[nodiscard]] quint64 generation() const noexcept { return generation_; }
    [[nodiscard]] bool hasOperationWorker() const noexcept { return operationWorker_ != nullptr; }
    [[nodiscard]] bool hasServerWorker() const noexcept { return serverWorker_ != nullptr; }

    // Diagnostic & Formatting helpers
    [[nodiscard]] static QString formatChannelError(io::ChannelErrorCode code, const QString& detail);
    [[nodiscard]] static QString formatChannelState(io::ChannelState state, ChannelRole role);

signals:
    void snapshotChanged(const SessionUiSnapshot& snapshot);
    void unexpectedDisconnectDetected();
    void errorOccurred(const QString& deviceHint, io::ChannelErrorCode code, const QString& message);
    void trafficOccurred(bool isTx, const QByteArray& data);
    void clientTrafficOccurred(bool isTx, const QByteArray& data, int clientId);
    void clientConnected(int clientId, const QString& peerInfo);
    void clientDisconnected(int clientId);
    void autoSendStateChanged(bool running);

public slots:
    // Slot receiving state changes from underlying workers or simulation tests
    void onWorkerStateChanged(io::ChannelState nextState, quint64 generation);

private slots:
    void onAutoSendTimeout();

private:
    io::ChannelOperationWorker* ensureOperationWorker();
    io::ServerChannelWorker* ensureServerWorker();

    void transitionTo(io::ChannelState nextState);
    void updateSnapshot(bool stateChangedFlag);

    io::ChannelState state_ = io::ChannelState::Closed;
    ChannelRole activeRole_ = ChannelRole::None;
    quint64 generation_ = 0;

    bool serverHasTargets_ = false;
    bool serverBroadcastMode_ = false;

    SessionUiSnapshot currentSnapshot_;

    // Lazy worker pairs
    std::shared_ptr<QThread> operationThread_;
    std::shared_ptr<io::ChannelOperationWorker> operationWorker_;

    std::shared_ptr<QThread> serverThread_;
    std::shared_ptr<io::ServerChannelWorker> serverWorker_;

    // Auto-send state
    QTimer* autoSendTimer_ = nullptr;
    std::function<QByteArray()> dataSupplier_;
    std::function<void(const QByteArray&)> sendAction_;
};

} // namespace ui::application
