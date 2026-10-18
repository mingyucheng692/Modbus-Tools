// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <gmock/gmock.h>
#include "ui/application/modbus/IModbusPageView.h"

namespace tests::mocks {

/// Role: GMock test double for IModbusPageView contract verification in headless tests.
/// Thread-safety: Thread-compatible test double.
/// Ownership: Instantiated and owned by test fixture.
class MockModbusPageView : public ::ui::application::modbus::IModbusPageView {
public:
    MOCK_METHOD(void, renderConnectionState, (::ui::application::modbus::SessionConnectionState), (override));
    MOCK_METHOD(void, updateAddressBase, (::modbus::address::AddressBase), (override));
    MOCK_METHOD(void, setProtocolType, (::modbus::parser::ProtocolType), (override));
    MOCK_METHOD(void, appendTrafficTelemetry, (const ::ui::common::TrafficEvent&), (override));
    MOCK_METHOD(void, updateMetrics, (int, int, int, int), (override));
    MOCK_METHOD(void, setExecutionGuards, (const ::ui::application::modbus::ExecutionGuardState&), (override));
    MOCK_METHOD(void, setLinkageState, (bool), (override));
    MOCK_METHOD(void, showStatusMessage, (const QString&, bool), (override));
};

} // namespace tests::mocks

