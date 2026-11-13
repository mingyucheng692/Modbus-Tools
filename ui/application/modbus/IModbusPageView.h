// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QString>
#include "ModbusTypes.h"
#include "modbus/base/ModbusAddressMapping.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "common/TrafficEvent.h"

namespace ui::application::modbus {

/// Role: Presentation composite state snapshot synchronizing guard permissions and running states.
/// Thread-safety: Thread-compatible plain data structure.
/// Ownership: Value object passed by const reference.
struct ExecutionGuardState {
    bool executeEnabled = false;
    bool pollingEnabled = false;
    bool pollingActive = false;
    bool rawSendEnabled = false;
    bool protocolSwitchEnabled = true;
    QString tooltipMessage;
};

/// Role: Pure virtual view contract for ModbusPage presentation and decoupled testing.
/// Thread-safety: Thread-affine (must be invoked on GUI main thread).
/// Ownership: Interface implemented by views::modbus::ModbusPage; observed by ModbusPagePresenter.
class IModbusPageView {
public:
    virtual ~IModbusPageView() noexcept = default;
    virtual void renderConnectionState(SessionConnectionState state) = 0;
    virtual void updateAddressBase(::modbus::address::AddressBase base) = 0;
    virtual void setProtocolType(::modbus::parser::ProtocolType protocol) = 0;
    virtual void appendTrafficTelemetry(const ::ui::common::TrafficEvent& event) = 0;
    virtual void updateMetrics(int tx, int rx, int err, int rttMs) = 0;
    virtual void setExecutionGuards(const ExecutionGuardState& state) = 0;
    virtual void setLinkageState(bool active) = 0;
    virtual void showStatusMessage(const QString& message, bool isError) = 0;
};

} // namespace ui::application::modbus

