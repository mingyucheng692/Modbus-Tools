/**
 * @file ModbusPagePresenter.h
 * @brief Presenter for ModbusPage — owns backend services and signal routing.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include "ModbusTypes.h"

namespace ui::widgets {
class BaseConnectionWidget;
class ControlWidget;
class FunctionWidget;
class TrafficMonitorWidget;
}

namespace ui::views::modbus { class ModbusPage; }

namespace ui::application::modbus {

class ModbusSessionPresenter;
class RequestSubmissionService;
class PollingController;
class TrafficLogController;
class RequestCoordinator;

/**
 * @brief Composition root for the Modbus page — owns backend services and
 *        orchestrates signal routing between them and the View.
 *
 * Replaces the former BaseModbusPage's role as composition root (ADR 0004).
 * The View (ModbusPage) creates UI widgets, then delegates service
 * creation and wiring to this Presenter via setup().
 *
 * Supports in-place protocol switching via switchMode(): tears down the
 * current services and rebuilds them with the new SessionMode.
 *
 * @thread Lives on the GUI thread. All methods must be called from the
 *         GUI thread.
 */
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
    /// @param newMode The target protocol mode.
    /// @param newConnectionWidget The connection widget for the new mode
    ///        (TCP widget for Tcp, serial widget for Rtu/Ascii).
    void switchMode(SessionMode newMode,
                    ui::widgets::BaseConnectionWidget* newConnectionWidget);

    // --- Delegating API for the View ---
    void requestConnect(const ModbusConnectionSpec& spec);
    void requestDisconnect();
    void updateSettings(const ModbusTimingParams& params);
    void setLinked(bool linked);
    [[nodiscard]] bool isSessionConnected() const;
    [[nodiscard]] bool isLinked() const;
    [[nodiscard]] ModbusSessionPresenter* sessionPresenter() const;

signals:
    /// Forwarded from RequestCoordinator (gated by linked_ state).
    void linkageDataReceived(const ::modbus::base::Pdu& pdu,
                             ::modbus::core::parser::ProtocolType protocol,
                             uint16_t addr);
    /// Forwarded from ControlWidget link toggle.
    void linkageToggled(bool active);
    /// Forwarded from ModbusSessionPresenter.
    void linkageSourceDisconnected();

private:
    void createServices();
    void wireConnections();
    void teardownServices();

    ui::views::modbus::ModbusPage* view_ = nullptr;
    SessionMode mode_;

    ModbusSessionPresenter* sessionPresenter_ = nullptr;
    RequestSubmissionService* requestService_ = nullptr;
    PollingController* pollingController_ = nullptr;
    TrafficLogController* trafficLogController_ = nullptr;
    RequestCoordinator* requestCoordinator_ = nullptr;

    ui::widgets::BaseConnectionWidget* connectionWidget_ = nullptr;
    ui::widgets::ControlWidget* controlWidget_ = nullptr;
    ui::widgets::FunctionWidget* functionWidget_ = nullptr;
    ui::widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;
    bool linked_ = false;
};

} // namespace ui::application::modbus
