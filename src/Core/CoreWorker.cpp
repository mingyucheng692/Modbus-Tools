#include "CoreWorker.h"
#include <QThread>
#include <QTimer>
#include "Logging/TrafficLog.h"

CoreWorker::CoreWorker(QObject* parent) : QObject(parent) {
    pollTimer_ = new QTimer(this);
    connect(pollTimer_, &QTimer::timeout, this, &CoreWorker::onPollTimeout);
}

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

#include <sstream>
#include <iomanip>

void CoreWorker::sendRequest(int slaveId, int funcCode, int startAddr, int count, const QString& dataHex) {
    if (!tcpChannel_ || tcpChannel_->state() != ChannelState::Open) {
        spdlog::warn("Core: Channel not open, cannot send request");
        return;
    }
    
    std::vector<uint8_t> data;
    if (!dataHex.isEmpty()) {
        std::stringstream ss(dataHex.toStdString());
        std::string item;
        while (ss >> item) {
            try {
                data.push_back(static_cast<uint8_t>(std::stoi(item, nullptr, 16)));
            } catch (...) {}
        }
    }
    
    // Check if ModbusTCP or RTU (Currently we only have ModbusTcpClient wired up fully in init)
    // Ideally we should switch between clients based on channel type.
    // For now assuming TCP for the screenshot context.
    
    if (modbusClient_) {
        modbusClient_->sendRequest(static_cast<uint8_t>(slaveId), 
                                   static_cast<Modbus::FunctionCode>(funcCode), 
                                   static_cast<uint16_t>(startAddr), 
                                   static_cast<uint16_t>(count), 
                                   data);
    }
    
    // Store for polling
    lastRequest_ = {slaveId, funcCode, startAddr, count, dataHex};
}

void CoreWorker::setPolling(bool enabled, int intervalMs) {
    if (enabled) {
        if (intervalMs < 10) intervalMs = 10;
        pollTimer_->start(intervalMs);
        spdlog::info("Core: Polling started, interval {}ms", intervalMs);
    } else {
        pollTimer_->stop();
        spdlog::info("Core: Polling stopped");
    }
}

void CoreWorker::setSimulation(int dropRate, int minDelay, int maxDelay) {
    if (tcpChannel_) {
        tcpChannel_->setSimulation(dropRate, minDelay, maxDelay);
        spdlog::info("Core: Simulation updated - Drop: {}%, Delay: {}-{}ms", dropRate, minDelay, maxDelay);
    }
}

void CoreWorker::sendRaw(const std::vector<uint8_t>& data) {
    if (!tcpChannel_ || tcpChannel_->state() != ChannelState::Open) {
        spdlog::warn("Core: Channel not open, cannot send raw data");
        return;
    }
    tcpChannel_->write(data);
}

void CoreWorker::onPollTimeout() {
    // Repeat last request
    if (tcpChannel_ && tcpChannel_->state() == ChannelState::Open) {
        sendRequest(lastRequest_.slaveId, lastRequest_.funcCode, 
                    lastRequest_.startAddr, lastRequest_.count, lastRequest_.dataHex);
    }
}
