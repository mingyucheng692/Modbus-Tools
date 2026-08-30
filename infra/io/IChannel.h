/**
 * @file IChannel.h
 * @brief Header file for IChannel.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include "core/Config.h"
#include <QByteArrayView>
#include <QString>
#include <QtGlobal>
#include <QMetaType>
#include <functional>

class QThread;

namespace io {

enum class ChannelKind { Tcp, Serial, Udp };
enum class ChannelState { Closed, Opening, Open, Closing, Error };
enum class SerialSignal { Dtr, Rts };

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

    ChannelError() = default;
    ChannelError(ChannelErrorCode c, QString msg)
        : code(c), message(std::move(msg)) {}

    explicit operator bool() const { return code != ChannelErrorCode::None; }
};

struct ChannelStats {
    quint64 bytesTx = 0;
    quint64 bytesRx = 0;
};

struct Timeouts {
    int readMs = config::Modbus::kDefaultTimeoutMs;
    int writeMs = config::Modbus::kDefaultTimeoutMs;
};

class IChannel {
public:
    using HandlerId = quint64;

    virtual ~IChannel() noexcept = default;
    virtual ChannelKind kind() const = 0;
    virtual ChannelState state() const = 0;
    virtual bool open() = 0;
    /// Moves the channel (and its underlying Qt I/O object) to the given
    /// thread. NOTE: this contract is bound to Qt's event-loop thread model
    /// — the I/O object's event-driven handlers (read/error/state) run on
    /// whatever QThread owns it. A future non-Qt backend cannot simply
    /// reimplement this method; the thread-affinity contract itself would
    /// need to be redesigned.
    virtual void moveToThread(QThread* thread) = 0;
    virtual void close() = 0;
    /// Transport-level liveness fact: the underlying link is currently
    /// usable (channel FSM reached Open). This is the LOWEST of the three
    /// connection vocabularies and is purely about the transport — it says
    /// nothing about Modbus session state. Compare:
    ///   - io::IChannel::isOpen()      (this) — transport link usable
    ///   - ModbusClient::isConnected()         — delegates to isOpen(); a
    ///     convenience alias for callers of the client facade
    ///   - ConnectionStateMachine Connected  — session-level fact: a session
    ///     was established over the transport and has not been declared
    ///     lost (passive channel loss / eviction). A half-open TCP socket
    ///     can have isOpen()==true while the FSM already reports Failed.
    /// isOpen() may also transiently be true after a passive loss until the
    /// channel's close handshake completes.
    virtual bool isOpen() const = 0;
    virtual void setTimeouts(const Timeouts& timeouts) = 0;
    virtual Timeouts timeouts() const = 0;
    virtual bool write(QByteArrayView data) = 0;
    virtual void setReadHandler(std::function<void(QByteArrayView)> handler) = 0;
    virtual void setErrorHandler(std::function<void(const ChannelError&)> handler) = 0;
    virtual void setWriteDrainedHandler(std::function<void()> handler) = 0;
    virtual HandlerId addStateHandler(std::function<void(ChannelState)> handler) = 0;
    virtual void removeStateHandler(HandlerId handlerId) = 0;
    // isTx: true=TX, false=RX
    virtual void setMonitor(std::function<void(bool isTx, const QByteArray&)> monitor) = 0;
    virtual ChannelStats stats() const = 0;
    /// Serial control signals (DTR, RTS). Returns false by default;
    /// SerialChannel overrides to forward to the underlying QSerialPort.
    virtual bool setSerialControl(SerialSignal signal, bool value) { return false; }
};

}

Q_DECLARE_METATYPE(io::ChannelState)
Q_DECLARE_METATYPE(io::ChannelErrorCode)
