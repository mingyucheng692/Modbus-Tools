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
    
    serialChannel_ = new SerialChannel(this);
    rtuClient_ = new Modbus::ModbusRtuClient(serialChannel_, this);
    
    auto wireChannel = [this](IChannel* channel, const QString& name) {
        connect(channel, &IChannel::opened, this, [this, name](){
            emit connectionStateChanged(true);
            spdlog::info("Core: {} Connection Established", name.toStdString());
        });
        
        connect(channel, &IChannel::closed, this, [this, name](){
            emit connectionStateChanged(false);
            spdlog::info("Core: {} Connection Closed", name.toStdString());
        });
        
        connect(channel, &IChannel::errorOccurred, this, [this, name](const QString& msg){
            emit errorOccurred(name + ": " + msg);
            spdlog::error("Core: {} Error: {}", name.toStdString(), msg.toStdString());
        });
        
        // Traffic Logging
        connect(channel, &IChannel::dataReceived, this, [](const std::vector<uint8_t>& data){
            RawFrame frame;
            frame.timestamp = std::chrono::system_clock::now();
            frame.direction = Direction::Rx;
            frame.data = data;
            TrafficLog::instance().addFrame(frame);
        });
        
        connect(channel, &IChannel::dataSent, this, [](const std::vector<uint8_t>& data){
            RawFrame frame;
            frame.timestamp = std::chrono::system_clock::now();
            frame.direction = Direction::Tx;
            frame.data = data;
            TrafficLog::instance().addFrame(frame);
        });
    };

    wireChannel(tcpChannel_, "TCP");
    wireChannel(serialChannel_, "Serial");

    emit workerReady();
}

void CoreWorker::cleanup() {
    spdlog::info("CoreWorker Cleanup");
    if (activeChannel_) {
        activeChannel_->close();
    }
}

void CoreWorker::testWorker() {
    spdlog::info("CoreWorker received test command in thread: {}", (uint64_t)QThread::currentThreadId());
    emit testResponse("Hello from Worker Thread");
}

void CoreWorker::connectTcp(const QString& ip, int port) {
    disconnect();
    tcpChannel_->setConnectionSettings(ip, port);
    activeChannel_ = tcpChannel_;
    tcpChannel_->open();
}

void CoreWorker::connectRtu(const QString& portName, int baudRate, int dataBits, int stopBits, int parity) {
    disconnect();
    serialChannel_->setConnectionSettings(portName, baudRate, dataBits, stopBits, parity);
    activeChannel_ = serialChannel_;
    serialChannel_->open();
}

void CoreWorker::connectSerial(const QString& portName, int baudRate, int dataBits, int stopBits, int parity) {
    // Same as RTU physically
    connectRtu(portName, baudRate, dataBits, stopBits, parity);
}

void CoreWorker::disconnect() {
    if (activeChannel_) {
        activeChannel_->close();
        activeChannel_ = nullptr;
    }
}

#include <sstream>
#include <iomanip>

void CoreWorker::sendRequest(int slaveId, int funcCode, int startAddr, int count, const QString& dataHex) {
    if (!activeChannel_ || activeChannel_->state() != ChannelState::Open) {
        spdlog::warn("Core: No active channel, cannot send request");
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
    
    if (activeChannel_ == tcpChannel_) {
        if (modbusClient_) {
            modbusClient_->sendRequest(static_cast<uint8_t>(slaveId), 
                                    static_cast<Modbus::FunctionCode>(funcCode), 
                                    static_cast<uint16_t>(startAddr), 
                                    static_cast<uint16_t>(count), 
                                    data);
        }
    } else if (activeChannel_ == serialChannel_) {
        if (rtuClient_) {
            rtuClient_->sendRequest(static_cast<uint8_t>(slaveId), 
                                    static_cast<Modbus::FunctionCode>(funcCode), 
                                    static_cast<uint16_t>(startAddr), 
                                    static_cast<uint16_t>(count), 
                                    data);
        }
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
    if (activeChannel_) {
        activeChannel_->setSimulation(dropRate, minDelay, maxDelay);
        spdlog::info("Core: Simulation updated - Drop: {}%, Delay: {}-{}ms", dropRate, minDelay, maxDelay);
    }
}

void CoreWorker::sendRaw(const std::vector<uint8_t>& data) {
    if (!activeChannel_ || activeChannel_->state() != ChannelState::Open) {
        spdlog::warn("Core: No active channel, cannot send raw data");
        return;
    }
    activeChannel_->write(data);
}

void CoreWorker::onPollTimeout() {
    // Repeat last request
    if (activeChannel_ && activeChannel_->state() == ChannelState::Open) {
        sendRequest(lastRequest_.slaveId, lastRequest_.funcCode, 
                    lastRequest_.startAddr, lastRequest_.count, lastRequest_.dataHex);
    }
}
