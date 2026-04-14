/**
 * @file ChannelBase.h
 * @brief Header file for ChannelBase.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "IChannel.h"
#include <atomic>
#include <mutex>
#include <vector>

namespace io {

class ChannelBase : public IChannel {
public:
    ChannelState state() const override;
    void moveToThread(QThread* thread) override;
    bool isOpen() const override;
    void setTimeouts(const Timeouts& timeouts) override;
    Timeouts timeouts() const override;
    void setReadHandler(std::function<void(QByteArrayView)> handler) override;
    void setErrorHandler(std::function<void(const QString&)> handler) override;
    void setWriteDrainedHandler(std::function<void()> handler) override;
    void setStateHandler(std::function<void(ChannelState)> handler) override;
    HandlerId addStateHandler(std::function<void(ChannelState)> handler) override;
    void removeStateHandler(HandlerId handlerId) override;
    void setMonitor(std::function<void(bool isTx, const QByteArray&)> monitor) override;
    ChannelStats stats() const override;

protected:
    void setState(ChannelState state);
    void addTx(qsizetype bytes);
    void addRx(qsizetype bytes);
    void emitRead(QByteArrayView data);
    void emitError(const QString& error);
    void emitWriteDrained();
    void emitMonitor(bool isTx, const QByteArray& data);

private:
    std::atomic<ChannelState> state_{ChannelState::Closed};
    Timeouts timeouts_{};
    ChannelStats stats_{};
    std::function<void(QByteArrayView)> readHandler_;
    std::function<void(const QString&)> errorHandler_;
    std::function<void()> writeDrainedHandler_;
    std::function<void(bool, const QByteArray&)> monitor_;
    std::mutex stateHandlersMutex_;
    HandlerId nextStateHandlerId_ = 1;
    std::vector<std::pair<HandlerId, std::function<void(ChannelState)>>> stateHandlers_;
};

}
