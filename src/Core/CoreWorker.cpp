#include "CoreWorker.h"
#include <QThread>
#include "Logging/TrafficLog.h"

CoreWorker::CoreWorker(QObject* parent) : QObject(parent) {}

CoreWorker::~CoreWorker() {
    spdlog::info("CoreWorker Destroyed in thread: {}", (uint64_t)QThread::currentThreadId());
}

void CoreWorker::init() {
    spdlog::info("CoreWorker Initialized in thread: {}", (uint64_t)QThread::currentThreadId());
    
    tcpChannel_ = new TcpChannel(this);
    modbusClient_ = new Modbus::ModbusTcpClient(tcpChannel_, this);
    
    connect(tcpChannel_, &TcpChannel::opened, this, [this](){
        emit connectionStateChanged(true);
        spdlog::info("Core: Connection Established");
    });
    
    connect(tcpChannel_, &TcpChannel::closed, this, [this](){
        emit connectionStateChanged(false);
        spdlog::info("Core: Connection Closed");
    });
    
    connect(tcpChannel_, &TcpChannel::errorOccurred, this, [this](const QString& msg){
        emit errorOccurred(msg);
        spdlog::error("Core: Channel Error: {}", msg.toStdString());
    });
    
    // Traffic Logging
    connect(tcpChannel_, &TcpChannel::dataReceived, this, [](const std::vector<uint8_t>& data){
        RawFrame frame;
        frame.timestamp = std::chrono::system_clock::now();
        frame.direction = Direction::Rx;
        frame.data = data;
        TrafficLog::instance().addFrame(frame);
    });
    
    connect(tcpChannel_, &TcpChannel::dataSent, this, [](const std::vector<uint8_t>& data){
        RawFrame frame;
        frame.timestamp = std::chrono::system_clock::now();
        frame.direction = Direction::Tx;
        frame.data = data;
        TrafficLog::instance().addFrame(frame);
    });

    emit workerReady();
}

void CoreWorker::cleanup() {
    spdlog::info("CoreWorker Cleanup");
    if (tcpChannel_) {
        tcpChannel_->close();
    }
}

void CoreWorker::testWorker() {
    spdlog::info("CoreWorker received test command in thread: {}", (uint64_t)QThread::currentThreadId());
    emit testResponse("Hello from Worker Thread");
}

void CoreWorker::connectTcp(const QString& ip, int port) {
    if (tcpChannel_) {
        tcpChannel_->setConnectionSettings(ip, port);
        tcpChannel_->open();
    }
}

void CoreWorker::disconnect() {
    if (tcpChannel_) {
        tcpChannel_->close();
    }
}
