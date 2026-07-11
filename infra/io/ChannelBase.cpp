/**
 * @file ChannelBase.cpp
 * @brief Implementation of ChannelBase.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ChannelBase.h"
#include <algorithm>

namespace io {

ChannelState ChannelBase::state() const
{
    return state_.load();
}

void ChannelBase::moveToThread(QThread* thread)
{
    Q_UNUSED(thread);
}

bool ChannelBase::isOpen() const
{
    return state_.load() == ChannelState::Open;
}

void ChannelBase::setTimeouts(const Timeouts& timeouts)
{
    timeouts_ = timeouts;
}

Timeouts ChannelBase::timeouts() const
{
    return timeouts_;
}

void ChannelBase::setReadHandler(std::function<void(QByteArrayView)> handler)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    readHandler_ = std::move(handler);
}

void ChannelBase::setErrorHandler(std::function<void(const QString&)> handler)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    errorHandler_ = std::move(handler);
}

void ChannelBase::setWriteDrainedHandler(std::function<void()> handler)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    writeDrainedHandler_ = std::move(handler);
}

IChannel::HandlerId ChannelBase::addStateHandler(std::function<void(ChannelState)> handler)
{
    if (!handler) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(stateHandlersMutex_);
    const HandlerId id = nextStateHandlerId_++;
    stateHandlers_.emplace_back(id, std::move(handler));
    return id;
}

void ChannelBase::removeStateHandler(HandlerId handlerId)
{
    if (handlerId == 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(stateHandlersMutex_);
    stateHandlers_.erase(
        std::remove_if(stateHandlers_.begin(),
                       stateHandlers_.end(),
                       [handlerId](const auto& entry) {
                           return entry.first == handlerId;
                       }),
        stateHandlers_.end());
}

void ChannelBase::setMonitor(std::function<void(bool, const QByteArray&)> monitor)
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    monitor_ = std::move(monitor);
}

ChannelStats ChannelBase::stats() const
{
    return stats_;
}

void ChannelBase::setState(ChannelState state)
{
    const ChannelState previous = state_.exchange(state);
    if (previous == state) {
        return;
    }

    std::vector<std::function<void(ChannelState)>> handlers;
    {
        std::lock_guard<std::mutex> lock(stateHandlersMutex_);
        handlers.reserve(stateHandlers_.size());
        for (const auto& entry : stateHandlers_) {
            if (entry.second) {
                handlers.push_back(entry.second);
            }
        }
    }

    for (const auto& handler : handlers) {
        handler(state);
    }
}

void ChannelBase::addTx(qsizetype bytes)
{
    if (bytes > 0) {
        stats_.bytesTx += static_cast<quint64>(bytes);
    }
}

void ChannelBase::addRx(qsizetype bytes)
{
    if (bytes > 0) {
        stats_.bytesRx += static_cast<quint64>(bytes);
    }
}

void ChannelBase::emitRead(QByteArrayView data)
{
    std::function<void(QByteArrayView)> handler;
    {
        std::lock_guard<std::mutex> lock(handlerMutex_);
        handler = readHandler_;
    }
    if (handler) {
        handler(data);
    }
}

void ChannelBase::emitError(const QString& error)
{
    std::function<void(const QString&)> handler;
    {
        std::lock_guard<std::mutex> lock(handlerMutex_);
        handler = errorHandler_;
    }
    if (handler) {
        handler(error);
    }
}

void ChannelBase::emitWriteDrained()
{
    std::function<void()> handler;
    {
        std::lock_guard<std::mutex> lock(handlerMutex_);
        handler = writeDrainedHandler_;
    }
    if (handler) {
        handler();
    }
}

void ChannelBase::emitMonitor(bool isTx, const QByteArray& data)
{
    std::function<void(bool, const QByteArray&)> handler;
    {
        std::lock_guard<std::mutex> lock(handlerMutex_);
        handler = monitor_;
    }
    if (handler) {
        handler(isTx, data);
    }
}

}
