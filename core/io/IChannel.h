#pragma once

#include "AppConstants.h"
#include <QByteArrayView>
#include <QString>
#include <QtGlobal>
#include <QMetaType>
#include <functional>

class QThread;

namespace io {

enum class ChannelKind { Tcp, Serial };
enum class ChannelState { Closed, Opening, Open, Closing, Error };

enum class ChannelErrorCode {
    None = 0,
    ConnectionFailed,
    Timeout,
    WriteFailed,
    ReadFailed,
    PortNotFound,
    PermissionDenied,
    ConnectionReset,
    Unknown
};

struct ChannelError {
    ChannelErrorCode code = ChannelErrorCode::None;
    QString message;
    QString deviceHint;

    ChannelError() = default;
    ChannelError(ChannelErrorCode c, QString msg, QString hint = {})
        : code(c), message(std::move(msg)), deviceHint(std::move(hint)) {}

    explicit operator bool() const { return code != ChannelErrorCode::None; }
};

struct ChannelStats {
    quint64 bytesTx = 0;
    quint64 bytesRx = 0;
};

struct Timeouts {
    int readMs = app::constants::Constants::Modbus::kDefaultTimeoutMs;
    int writeMs = app::constants::Constants::Modbus::kDefaultTimeoutMs;
};

class IChannel {
public:
    using HandlerId = quint64;

    virtual ~IChannel() = default;
    virtual ChannelKind kind() const = 0;
    virtual ChannelState state() const = 0;
    virtual bool open() = 0;
    virtual void moveToThread(QThread* thread) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual void setTimeouts(const Timeouts& timeouts) = 0;
    virtual Timeouts timeouts() const = 0;
    virtual bool write(QByteArrayView data) = 0;
    virtual void setReadHandler(std::function<void(QByteArrayView)> handler) = 0;
    virtual void setErrorHandler(std::function<void(const QString&)> handler) = 0;
    virtual void setWriteDrainedHandler(std::function<void()> handler) = 0;
    // Compatibility-only API: replaces all current state subscribers.
    virtual void setStateHandler(std::function<void(ChannelState)> handler) = 0;
    virtual HandlerId addStateHandler(std::function<void(ChannelState)> handler) = 0;
    virtual void removeStateHandler(HandlerId handlerId) = 0;
    // isTx: true=TX, false=RX
    virtual void setMonitor(std::function<void(bool isTx, const QByteArray&)> monitor) = 0;
    virtual ChannelStats stats() const = 0;
};

}

Q_DECLARE_METATYPE(io::ChannelState)
Q_DECLARE_METATYPE(io::ChannelErrorCode)
