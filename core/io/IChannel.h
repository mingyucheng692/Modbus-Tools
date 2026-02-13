#pragma once

#include <QByteArrayView>
#include <QString>
#include <QtGlobal>
#include <functional>

namespace io {

enum class ChannelKind { Tcp, Serial };
enum class ChannelState { Closed, Opening, Open, Closing, Error };

struct ChannelStats {
    quint64 bytesTx = 0;
    quint64 bytesRx = 0;
};

struct Timeouts {
    int readMs = 1000;
    int writeMs = 1000;
};

class IChannel {
public:
    virtual ~IChannel() = default;
    virtual ChannelKind kind() const = 0;
    virtual ChannelState state() const = 0;
    virtual bool open() = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual void setTimeouts(const Timeouts& timeouts) = 0;
    virtual Timeouts timeouts() const = 0;
    virtual bool write(QByteArrayView data) = 0;
    virtual void setReadHandler(std::function<void(QByteArrayView)> handler) = 0;
    virtual void setErrorHandler(std::function<void(const QString&)> handler) = 0;
    // isTx: true=TX, false=RX
    virtual void setMonitor(std::function<void(bool isTx, const QByteArray&)> monitor) = 0;
    virtual ChannelStats stats() const = 0;
};

}
