/**
 * @file TcpStateTransition.h
 * @brief Stateless TCP connection state transition computation.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "../../infra/io/IChannel.h"

namespace ui::common {

struct TcpConnectionStateTransition {
    bool setConnected = false;
    bool setDisconnected = false;
    bool clearSuppressDisconnectAlert = false;
    bool showDisconnectAlert = false;
};

/// Compute the side-effect flags for a TCP connection state change.
/// Replaces the former TcpConnectionStateCoordinator static class.
inline TcpConnectionStateTransition computeTcpStateTransition(io::ChannelState state,
                                                             bool wasConnected,
                                                             bool suppressDisconnectAlert) {
    TcpConnectionStateTransition result;
    if (state == io::ChannelState::Open) {
        result.setConnected = !wasConnected;
        result.clearSuppressDisconnectAlert = true;
        return result;
    }
    if (state == io::ChannelState::Closed && wasConnected) {
        result.setDisconnected = true;
        result.showDisconnectAlert = !suppressDisconnectAlert;
    }
    return result;
}

} // namespace ui::common
