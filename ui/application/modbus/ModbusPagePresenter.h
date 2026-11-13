// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QPointer>
#include <memory>
#include <optional>
#include "ModbusTypes.h"
#include "ModbusExecuteSpec.h"
#include "IModbusPageView.h"

namespace modbus::session { struct ModbusResponse; }

namespace ui::application::modbus {

class ModbusSessionPresenter;
class RequestSubmissionService;
class PollingController;
class TrafficLogController;

/// Role: Composition root for Modbus page — orchestrates backend services and routes use cases with IModbusPageView.
/// Thread-safety: Single-threaded (bound to GUI main thread).
/// Ownership: Owned by QObject parent (ModbusPage); observes IModbusPageView via non-owning pointer.
class ModbusPagePresenter : public QObject {
    Q_OBJECT

public:
    explicit ModbusPagePresenter(IModbusPageView* view,
                                 SessionMode mode,
                                 QObject* parent = nullptr);
    ~ModbusPagePresenter() noexcept override;

    void detachView() noexcept;
    [[nodiscard]] IModbusPageView* view() const noexcept { return view_; }

    void switchMode(SessionMode newMode);

    // --- Backend connection & session API ---
    void requestConnect(const ModbusConnectionSpec& spec);
    void requestDisconnect();
    void updateSettings(const ModbusTimingParams& params);
    [[nodiscard]] const ModbusTimingParams& timingParams() const noexcept { return timingParams_; }
    [[nodiscard]] bool hasTimingParams() const noexcept { return hasTimingParams_; }
    [[nodiscard]] SessionMode mode() const noexcept { return mode_; }
    [[nodiscard]] bool isSwitchingMode() const noexcept { return isSwitchingMode_; }

    void setLinked(bool linked);
    [[nodiscard]] bool isLinked() const noexcept;
    [[nodiscard]] ModbusSessionPresenter* sessionPresenter() const noexcept;
    [[nodiscard]] PollingController* pollingController() const noexcept;
    [[nodiscard]] RequestSubmissionService* requestService() const noexcept;
    [[nodiscard]] TrafficLogController* trafficLogController() const noexcept;

    void syncWidgetGuards(SessionConnectionState state);
    [[nodiscard]] bool ensureConnected();

    void setTrafficLogControllerForTest(TrafficLogController* controller);

    [[nodiscard]] static QString formatModbusError(const ::modbus::session::ModbusResponse& response);

public slots:
    void executeRequest(const ModbusExecuteSpec& spec);
    void requestExecute(const ModbusExecuteSpec& spec) { executeRequest(spec); }
    void togglePolling(bool start, const ModbusExecuteSpec& spec, bool userConfirmedWritePoll = false);
    void requestPollToggle(bool start, int intervalMs, const ModbusExecuteSpec& spec, bool userConfirmedWritePoll = false) {
        changePollInterval(intervalMs);
        togglePolling(start, spec, userConfirmedWritePoll);
    }
    void changePollInterval(int intervalMs);
    void handleLogMessage(const QString& message, bool isError);
    void setRawFramesEnabled(bool enabled);

signals:
    void linkageDataReceived(const ::modbus::base::Pdu& pdu,
                             ::modbus::parser::ProtocolType protocol,
                             uint16_t addr);
    void linkageToggled(bool active);
    void linkageSourceDisconnected();
    void sessionConnectionLostAlert();

private:
    void createServices();
    void wireConnections();
    void teardownServices();
    void onStackReleasedForSwitch();
    void onStackReleaseTimedOutForSwitch(const QString& message);

    void handleRawExecute(const ModbusExecuteSpec& spec);
    void handleStandardExecute(const ModbusExecuteSpec& spec);
    void logExecuteSubmission(const ModbusExecuteSpec& spec, RequestKind kind, TraceId traceId);
    [[nodiscard]] bool validatePollingPreconditions(const ModbusExecuteSpec& spec, bool userConfirmedWritePoll);
    void handleRequestFinished(int requestId,
                               const ::modbus::session::ModbusResponse& response);

    void recordTx();
    void recordRx(int rttMs);
    void recordError();
    void notifyMetrics();

    IModbusPageView* view_ = nullptr;
    SessionMode mode_;
    SessionMode pendingMode_{};
    bool isSwitchingMode_ = false;
    ModbusTimingParams timingParams_{};
    bool hasTimingParams_ = false;

    ModbusSessionPresenter* sessionPresenter_ = nullptr;
    std::unique_ptr<RequestSubmissionService> requestService_;
    PollingController* pollingController_ = nullptr;
    QPointer<TrafficLogController> trafficLogController_;
    std::optional<SessionConnectionState> lastSyncedState_ = std::nullopt;
    bool lastPollingActive_ = false;
    bool lastProtocolSwitchEnabled_ = true;

    bool linked_ = false;
    int txCount_ = 0;
    int rxCount_ = 0;
    int errCount_ = 0;
    int lastRttMs_ = 0;
};

} // namespace ui::application::modbus
