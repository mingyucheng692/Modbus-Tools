#pragma once

#include "IChannel.h"

namespace io {

class ChannelBase : public IChannel {
public:
    ChannelState state() const override;
    bool isOpen() const override;
    void setTimeouts(const Timeouts& timeouts) override;
    Timeouts timeouts() const override;
    void setReadHandler(std::function<void(QByteArrayView)> handler) override;
    void setErrorHandler(std::function<void(const QString&)> handler) override;
    ChannelStats stats() const override;

protected:
    void setState(ChannelState state);
    void addTx(qsizetype bytes);
    void addRx(qsizetype bytes);
    void emitRead(QByteArrayView data);
    void emitError(const QString& error);

private:
    ChannelState state_{ChannelState::Closed};
    Timeouts timeouts_{};
    ChannelStats stats_{};
    std::function<void(QByteArrayView)> readHandler_;
    std::function<void(const QString&)> errorHandler_;
};

}
