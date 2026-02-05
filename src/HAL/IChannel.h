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

signals:
    void opened();
    void closed();
    void errorOccurred(const QString& msg);
    void dataReceived(const std::vector<uint8_t>& data);
};
