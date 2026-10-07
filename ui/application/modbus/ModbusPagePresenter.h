// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QPointer>
#include <memory>
#include <optional>
#include "ModbusTypes.h"

namespace ui::widgets {
class BaseConnectionWidget;
class ControlWidget;
class FunctionWidget;
class TrafficMonitorWidget;
}

namespace ui::views::modbus { class ModbusPage; }
namespace modbus::session { struct ModbusResponse; }

namespace ui::application::modbus {

class ModbusSessionPresenter;
class RequestSubmissionService;
class PollingController;
class TrafficLogController;

// Role: Composition root for Modbus page — orchestrates backend services and signal routing with the View.
// Thread-safety: Single-threaded (bound to GUI thread).
// Ownership: Owned by QObject parent (ModbusPage); owns child presenters and services.
class ModbusPagePresenter : public QObject {
    Q_OBJECT

public:
    explicit ModbusPagePresenter(ui::views::modbus::ModbusPage* view,
                                 SessionMode mode,
                                 QObject* parent = nullptr);
    ~ModbusPagePresenter() noexcept override;

    /// Creates backend services and wires them to the provided widgets.
    /// Must be called exactly once after the View has created its widgets.
    void setup(ui::widgets::BaseConnectionWidget* connectionWidget,
               ui::widgets::ControlWidget* controlWidget,
               ui::widgets::FunctionWidget* functionWidget,
               ui::widgets::TrafficMonitorWidget* trafficMonitor);

    /// Switches the active protocol. Disconnects first if connected, then
    /// tears down the old services and rebuilds them with the new mode.

    void switchMode(SessionMode newMode,
                    ui::widgets::BaseConnectionWidget* newConnectionWidget);

    // --- API for the View ---
    void requestConnect(const ModbusConnectionSpec& spec);
    void requestDisconnect();
    void updateSettings(const ModbusTimingParams& params);

    // setLinked() additionally mirrors the link flag into this presenter so
    // linkageDataReceived() can be gated without querying the session.
    void setLinked(bool linked);
    // isLinked() reports this presenter's mirrored flag, not the session's.
    [[nodiscard]] bool isLinked() const;
    [[nodiscard]] ModbusSessionPresenter* sessionPresenter() const;

    /// State-driven guarding entry (test seam & session connection sink).
    void syncWidgetGuards(SessionConnectionState state);
    [[nodiscard]] bool ensureConnected();

    /// Test seam for verifying QPointer lifecycle & null-fallback behavior.
    void setTrafficLogControllerForTest(TrafficLogController* controller);

    /// Formats structured ModbusError into localized user-facing UI text.
    [[nodiscard]] static QString formatModbusError(const ::modbus::session::ModbusResponse& response);

signals:
    /// Forwarded on successful request completion from session (gated by linked_ state).
    void linkageDataReceived(const ::modbus::base::Pdu& pdu,
                             ::modbus::parser::ProtocolType protocol,
                             uint16_t addr);
    /// Forwarded from ControlWidget link toggle.
    void linkageToggled(bool active);
    /// Forwarded from ModbusSessionPresenter.
    void linkageSourceDisconnected();

private:
    void createServices();
    void wireConnections();
    void teardownServices();
    void onStackReleasedForSwitch();

    void handleReadRequest(uint8_t fc, int addr, int qty, int slaveId);
    void handleWriteRequest(uint8_t fc, int addr, const QString& dataStr,
                            const QString& fmt, int slaveId, int quantity);
    void handleRawSendRequest(const QByteArray& data);
    void handlePollRequest(uint8_t fc, int addr, int qty, int intervalMs);
    void handlePollStartRequest(uint8_t fc, int addr, int qty, int intervalMs);
    void handlePollStopRequest();
    void handleRequestFinished(int requestId,
                               const ::modbus::session::ModbusResponse& response);

    ui::views::modbus::ModbusPage* view_ = nullptr;
    SessionMode mode_;
    SessionMode pendingMode_{};
    ui::widgets::BaseConnectionWidget* pendingConnectionWidget_ = nullptr;

    ModbusSessionPresenter* sessionPresenter_ = nullptr;
    // Plain C++ (non-QObject): unique_ptr ownership; must never be
    // deleteLater()'d. Destroyed in teardownServices() before the QObject
    // services that hold raw pointers into it are recreated.
    std::unique_ptr<RequestSubmissionService> requestService_;
    PollingController* pollingController_ = nullptr;
    QPointer<TrafficLogController> trafficLogController_;
    std::optional<SessionConnectionState> lastSyncedState_ = std::nullopt;

    ui::widgets::BaseConnectionWidget* connectionWidget_ = nullptr;
    ui::widgets::ControlWidget* controlWidget_ = nullptr;
    ui::widgets::FunctionWidget* functionWidget_ = nullptr;
    ui::widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
    bool linked_ = false;
};

} // namespace ui::application::modbus
