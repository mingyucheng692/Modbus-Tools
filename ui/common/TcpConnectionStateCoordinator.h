#pragma once

#include "../../core/io/IChannel.h"

namespace ui::common {

struct TcpConnectionStateTransition {
    bool setConnected = false;
    bool setDisconnected = false;
    bool clearSuppressDisconnectAlert = false;
    bool showDisconnectAlert = false;
};

class TcpConnectionStateCoordinator {
public:
    static TcpConnectionStateTransition transition(io::ChannelState state,
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
};

} // namespace ui::common
