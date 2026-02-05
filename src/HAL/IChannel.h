#pragma once
#include <QObject>
#include <QString>
#include <vector>
#include <span>

enum class ChannelState {
    Closed,
    Opening,
    Open,
    Closing,
    Error
};

class IChannel : public QObject {
    Q_OBJECT
public:
    explicit IChannel(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IChannel() = default;

    virtual void open() = 0;
    virtual void close() = 0;
    virtual void write(const std::vector<uint8_t>& data) = 0;
    virtual void write(const char* data, size_t size) = 0;
    
    virtual ChannelState state() const = 0;
    virtual QString errorString() const = 0;

    // Simulation Settings
    void setSimulation(int dropRate, int minDelayMs, int maxDelayMs) {
        simDropRate_ = dropRate;
        simMinDelay_ = minDelayMs;
        simMaxDelay_ = maxDelayMs;
    }

signals:
    void opened();
    void closed();
    void errorOccurred(const QString& msg);
    void dataReceived(const std::vector<uint8_t>& data);
    void dataSent(const std::vector<uint8_t>& data);

protected:
    int simDropRate_ = 0; // 0-100%
    int simMinDelay_ = 0;
    int simMaxDelay_ = 0;
    
    bool shouldDrop() const {
        if (simDropRate_ <= 0) return false;
        return (rand() % 100) < simDropRate_;
    }
    
    int getDelay() const {
        if (simMaxDelay_ <= 0) return 0;
        if (simMinDelay_ >= simMaxDelay_) return simMinDelay_;
        return simMinDelay_ + (rand() % (simMaxDelay_ - simMinDelay_));
    }
};
