#pragma once

#include "IChannel.h"

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
    void setStateHandler(std::function<void(ChannelState)> handler) override;
    void setMonitor(std::function<void(bool isTx, const QByteArray&)> monitor) override;
    ChannelStats stats() const override;

protected:
    void setState(ChannelState state);
    void addTx(qsizetype bytes);
    void addRx(qsizetype bytes);
    void emitRead(QByteArrayView data);
    void emitError(const QString& error);
    void emitMonitor(bool isTx, const QByteArray& data);

private:
    ChannelState state_{ChannelState::Closed};
    Timeouts timeouts_{};
    ChannelStats stats_{};
    std::function<void(QByteArrayView)> readHandler_;
    std::function<void(const QString&)> errorHandler_;
    std::function<void(ChannelState)> stateHandler_;
    std::function<void(bool, const QByteArray&)> monitor_;
};

}
