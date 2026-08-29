/**
 * @file ChannelOperationWorker.cpp
 * @brief Implementation of ChannelOperationWorker.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "ChannelOperationWorker.h"
#include "core/Config.h"
#include <spdlog/spdlog.h>
#include "TcpChannel.h"
#include "SerialChannel.h"
#include "UdpChannel.h"
#include <QThread>
#include <QMetaObject>
#include <QMetaType>

namespace io {

ChannelOperationWorker::ChannelOperationWorker(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<io::ChannelState>("io::ChannelState");
    qRegisterMetaType<io::ChannelErrorCode>("io::ChannelErrorCode");
}

ChannelOperationWorker::~ChannelOperationWorker()
{
    cleanupChannel();
}

void ChannelOperationWorker::openTcp(const QString& ip, int port, quint64 generation)
{
    cleanupChannel();
    channelGeneration_ = generation;
    deviceHint_ = QString("%1:%2").arg(ip).arg(port);

    auto tcp = std::make_shared<TcpChannel>();
    tcp->setEndpoint(ip, port);
    channel_ = tcp;

    setupChannel();

    if (!channel_->open()) {
        emitError("Failed to open TCP connection");
    }
}

void ChannelOperationWorker::openSerial(const SerialConfig& config, quint64 generation)
{
    cleanupChannel();
    channelGeneration_ = generation;
    deviceHint_ = config.portName;

    auto serial = std::make_shared<SerialChannel>();
    serial->setConfig(config);
    channel_ = serial;

    setupChannel();

    if (!channel_->open()) {
        emitError("Failed to open serial port");
    }
}

void ChannelOperationWorker::openUdp(const QString& localIp, int localPort,
                                      const QString& remoteIp, int remotePort)
{
    cleanupChannel();
    deviceHint_ = QString("UDP %1:%2").arg(localIp).arg(localPort);

    auto udp = std::make_shared<UdpChannel>();
    udp->setBindEndpoint(localIp, localPort);
    udp->setRemotePeer(remoteIp, remotePort);
    channel_ = udp;

    setupChannel();

    if (!channel_->open()) {
        emitError("Failed to open UDP channel");
    }
}

void ChannelOperationWorker::close()
{
    if (channel_) {
        channel_->close();
    }
}

void ChannelOperationWorker::write(const QByteArray& data)
{
    if (channel_ && channel_->isOpen()) {
        if (channel_->write(data)) {
            emit bytesQueued(data.size());
        } else {
            emitError("Write failed");
        }
    } else {
        emitError("Channel not open");
    }
}

void ChannelOperationWorker::setDtr(bool set)
{
    if (channel_) {
        channel_->setSerialControl(io::SerialSignal::Dtr, set);
    }
}

void ChannelOperationWorker::setRts(bool set)
{
    if (channel_) {
        channel_->setSerialControl(io::SerialSignal::Rts, set);
    }
}

void ChannelOperationWorker::setupChannel()
{
    if (!channel_) return;

    channel_->setErrorHandler([this](const QString& err) {
        emitError(err);
    });

    channel_->setMonitor([this](bool isTx, const QByteArray& data) {
        emit monitor(isTx, data);
    });

    stateHandlerId_ = channel_->addStateHandler([this](ChannelState state) {
        emit stateChanged(state);
        emit stateChangedWithGeneration(state, channelGeneration_);
    });
}

void ChannelOperationWorker::cleanupChannel()
{
    if (channel_) {
        // Order matters: removeStateHandler() BEFORE close(). close()
        // synchronously fires Closing/Closed through the state handlers;
        // with the handler still attached, a torn-down channel would emit
        // stateChangedWithGeneration with the OLD generation after the new
        // open*() has already bumped channelGeneration_, letting stale
        // events race the fresh connect attempt on the consumer side.
        channel_->removeStateHandler(stateHandlerId_);
        stateHandlerId_ = 0;
        channel_->close();
        channel_->setReadHandler(nullptr);
        channel_->setErrorHandler(nullptr);
        channel_->setMonitor(nullptr);
        channel_.reset();
    }
    deviceHint_.clear();
}

void ChannelOperationWorker::emitError(const QString& error)
{
    emit channelErrorOccurred(deviceHint_, error);
}

} // namespace io
