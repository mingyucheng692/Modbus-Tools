/**
 * @file IConnectionWidget.h
 * @brief Interface for connection widgets, decoupling Presenter from QWidget.
 *
 * Copyright (c) 2025 - present mingyecheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

namespace ui::widgets {

/**
 * @brief Connection display states shared by all connection widget implementations.
 */
enum class DisplayState {
    Disconnected = 0,
    Connecting,
    TransportConnected,
    Connected,
    Disconnecting,
    Listening,
    Bound
};

/**
 * @brief Abstract interface for connection widgets.
 *
 * Implemented by BaseConnectionWidget (and its subclasses) so that
 * ModbusSessionPresenter can drive display-state transitions without
 * qobject_cast downcasts or a concrete QWidget dependency.
 */
class IConnectionWidget {
public:
    virtual ~IConnectionWidget() = default;

    /// Update the widget's visual state to reflect a connection lifecycle phase.
    virtual void setDisplayState(DisplayState state) = 0;

    /// Toggle the connected/disconnected visual representation.
    virtual void setConnected(bool connected) = 0;
};

} // namespace ui::widgets
