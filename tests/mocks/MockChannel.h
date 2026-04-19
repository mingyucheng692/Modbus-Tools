#pragma once

#include <gmock/gmock.h>
#include "io/IChannel.h"

namespace io {

class MockChannel : public IChannel {
public:
    MOCK_METHOD(ChannelKind, kind, (), (const, override));
    MOCK_METHOD(ChannelState, state, (), (const, override));
    MOCK_METHOD(bool, open, (), (override));
    MOCK_METHOD(void, moveToThread, (QThread*), (override));
    MOCK_METHOD(void, close, (), (override));
    MOCK_METHOD(bool, isOpen, (), (const, override));
    MOCK_METHOD(void, setTimeouts, (const Timeouts&), (override));
    MOCK_METHOD(Timeouts, timeouts, (), (const, override));
    MOCK_METHOD(bool, write, (QByteArrayView), (override));
    MOCK_METHOD(void, setReadHandler, (std::function<void(QByteArrayView)>), (override));
    MOCK_METHOD(void, setErrorHandler, (std::function<void(const QString&)>), (override));
    MOCK_METHOD(void, setWriteDrainedHandler, (std::function<void()>), (override));
    MOCK_METHOD(void, setStateHandler, (std::function<void(ChannelState)>), (override));
    MOCK_METHOD(HandlerId, addStateHandler, (std::function<void(ChannelState)>), (override));
    MOCK_METHOD(void, removeStateHandler, (HandlerId), (override));
    MOCK_METHOD(void, setMonitor, ((std::function<void(bool, const QByteArray&)>)), (override));
    MOCK_METHOD(ChannelStats, stats, (), (const, override));
};

} // namespace io
