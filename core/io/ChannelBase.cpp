#include "ChannelBase.h"

namespace io {

ChannelState ChannelBase::state() const
{
    return state_;
}

void ChannelBase::moveToThread(QThread* thread)
{
    Q_UNUSED(thread);
}

bool ChannelBase::isOpen() const
{
    return state_ == ChannelState::Open;
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
    readHandler_ = std::move(handler);
}

void ChannelBase::setErrorHandler(std::function<void(const QString&)> handler)
{
    errorHandler_ = std::move(handler);
}

void ChannelBase::setStateHandler(std::function<void(ChannelState)> handler)
{
    stateHandler_ = std::move(handler);
}

void ChannelBase::setMonitor(std::function<void(bool, const QByteArray&)> monitor)
{
    monitor_ = std::move(monitor);
}

ChannelStats ChannelBase::stats() const
{
    return stats_;
}

void ChannelBase::setState(ChannelState state)
{
    if (state_ != state) {
        state_ = state;
        if (stateHandler_) {
            stateHandler_(state);
        }
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
    if (readHandler_) {
        readHandler_(data);
    }
}

void ChannelBase::emitError(const QString& error)
{
    if (errorHandler_) {
        errorHandler_(error);
    }
}

void ChannelBase::emitMonitor(bool isTx, const QByteArray& data)
{
    if (monitor_) {
        monitor_(isTx, data);
    }
}

}
