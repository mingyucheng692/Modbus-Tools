#pragma once
#include <QObject>
#include <spdlog/sinks/base_sink.h>
#include <mutex>
#include <QString>

// Base class for Signals to avoid MOC issues with templates
class QtLogSinkBase : public QObject {
    Q_OBJECT
public:
    explicit QtLogSinkBase(QObject* parent = nullptr) : QObject(parent) {}
signals:
    void logReceived(const QString& msg, int level);
};

// Spdlog sink implementation
template<typename Mutex>
class QtLogSink : public spdlog::sinks::base_sink<Mutex> {
public:
    QtLogSink(QtLogSinkBase* base) : base_(base) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        if (!base_) return;
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
        QString qmsg = QString::fromUtf8(formatted.data(), static_cast<int>(formatted.size()));
        // Emit signal (Thread safe because it's a signal)
        emit base_->logReceived(qmsg, static_cast<int>(msg.level));
    }

    void flush_() override {}

private:
    QtLogSinkBase* base_;
};
