#include "GenericIoWorker.h"
#include "TcpChannel.h"
#include "SerialChannel.h"
#include <QThread>
#include <QMetaObject>
#include <QMetaType>

namespace io {

GenericIoWorker::GenericIoWorker(QObject* parent)
    : QObject(parent)
{
    qRegisterMetaType<io::ChannelState>("io::ChannelState");
}

GenericIoWorker::~GenericIoWorker()
{
    cleanupChannel();
}

void GenericIoWorker::openTcp(const QString& ip, int port)
{
    cleanupChannel();

    auto tcp = std::make_shared<TcpChannel>();
    tcp->setEndpoint(ip, port);
    channel_ = tcp;

    setupChannel();

    if (!channel_->open()) {
        // If open failed synchronously
        emit errorOccurred("Failed to open TCP connection");
        // stateChanged will be handled by setStateHandler if it was set
    }
}

void GenericIoWorker::openSerial(const SerialConfig& config)
{
    cleanupChannel();

    auto serial = std::make_shared<SerialChannel>();
    serial->setConfig(config);
    channel_ = serial;

    setupChannel();

    if (!channel_->open()) {
        emit errorOccurred("Failed to open serial port");
    }
}

void GenericIoWorker::close()
{
    if (channel_) {
        channel_->close();
    }
}

void GenericIoWorker::write(const QByteArray& data)
{
    if (channel_ && channel_->isOpen()) {
        if (channel_->write(data)) {
            emit bytesWritten(data.size());
        } else {
            emit errorOccurred("Write failed");
        }
    } else {
        emit errorOccurred("Channel not open");
    }
}

void GenericIoWorker::setDtr(bool set)
{
    if (auto serial = std::dynamic_pointer_cast<SerialChannel>(channel_)) {
        serial->setDtr(set);
    }
}

void GenericIoWorker::setRts(bool set)
{
    if (auto serial = std::dynamic_pointer_cast<SerialChannel>(channel_)) {
        serial->setRts(set);
    }
}

void GenericIoWorker::setupChannel()
{
    if (!channel_) return;

    channel_->setErrorHandler([this](const QString& err) {
        emit errorOccurred(err);
    });

    channel_->setMonitor([this](bool isTx, const QByteArray& data) {
        emit monitor(isTx, data);
    });

    channel_->setStateHandler([this](ChannelState state) {
        emit stateChanged(state);
    });
}

void GenericIoWorker::cleanupChannel()
{
    if (channel_) {
        channel_->close();
        channel_->setReadHandler(nullptr);
        channel_->setErrorHandler(nullptr);
        channel_->setMonitor(nullptr);
        channel_->setStateHandler(nullptr);
        channel_.reset();
    }
}

} // namespace io
