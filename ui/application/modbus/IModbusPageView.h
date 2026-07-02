/**
 * @file IModbusPageView.h
 * @brief View interface for ModbusPagePresenter (ADR 0004 MVP pattern).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QByteArray>

namespace ui::application::modbus {

/**
 * @brief Interface implemented by the Modbus page View.
 *
 * The Presenter calls these methods to drive View rendering. The View calls
 * Presenter methods for business decisions. This inverts the dependency so
 * the Presenter can be unit-tested with a FakeView.
 */
class IModbusPageView {
public:
    virtual ~IModbusPageView() = default;

    /// Called by the Presenter when a raw frame is sent (isTx=true) or
    /// received (isTx=false). The View renders it in the traffic monitor.
    virtual void appendTrafficData(bool isTx, const QByteArray& data) = 0;
};

} // namespace ui::application::modbus
