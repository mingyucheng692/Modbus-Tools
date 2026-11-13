// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QIODevice>
#include <QMetaObject>
#include <QSignalSpy>
#include <QTimer>

#include <functional>

#include "infra/io/BufferedWritingChannel.h"

namespace {

class FakeIoDevice : public QIODevice {
    Q_OBJECT
public:
    explicit FakeIoDevice(QObject* parent = nullptr) : QIODevice(parent) {}

    bool open(OpenMode mode) override {
        return QIODevice::open(mode);
    }

    qint64 bytesToWrite() const override {
        return bufferedBytes_;
    }

    void setFailNextWrite(bool fail, const QString& err = QStringLiteral("Simulated I/O error")) {
        failWrite_ = fail;
        if (fail) {
            setErrorString(err);
        }
    }

    void setMaxBufferedBytes(qint64 limit) {
        maxBufferedBytes_ = limit;
    }

    void simulateBytesFlushed(qint64 bytes) {
        const qint64 drained = std::min(bytes, bufferedBytes_);
        bufferedBytes_ -= drained;
        emit bytesWritten(drained);
    }

    const QByteArray& writtenData() const { return written_; }
    int writeCallCount() const { return writeCalls_; }

protected:
    qint64 readData(char*, qint64) override { return 0; }

    qint64 writeData(const char* data, qint64 maxSize) override {
        ++writeCalls_;
        if (failWrite_) {
            return -1;
        }
        qint64 allowed = maxSize;
        if (maxBufferedBytes_ >= 0) {
            allowed = std::min(allowed, std::max<qint64>(0, maxBufferedBytes_ - bufferedBytes_));
        }
        if (allowed <= 0) {
            return 0;
        }
        written_.append(data, static_cast<qsizetype>(allowed));
        bufferedBytes_ += allowed;
        return allowed;
    }

private:
    QByteArray written_;
    qint64 bufferedBytes_ = 0;
    qint64 maxBufferedBytes_ = -1;
    bool failWrite_ = false;
    int writeCalls_ = 0;
};

class TestBufferedChannel : public io::BufferedWritingChannel {
public:
    TestBufferedChannel() {
        dev_.open(QIODevice::ReadWrite | QIODevice::Unbuffered);
        QObject::connect(&dev_, &QIODevice::bytesWritten, &dev_, [this](qint64 bytes) {
            onBytesWritten(bytes);
        }, Qt::QueuedConnection);
        setState(io::ChannelState::Open);
    }

    io::ChannelKind kind() const override { return io::ChannelKind::Tcp; }

    bool open() override {
        if (!dev_.isOpen()) {
            dev_.open(QIODevice::ReadWrite | QIODevice::Unbuffered);
        }
        setClosing(false);
        resetWriteState();
        setState(io::ChannelState::Open);
        return true;
    }

    void close() override {
        setClosing(true);
        resetWriteState();
        disarmWriteTimeout();
        if (dev_.isOpen()) {
            dev_.close();
        }
        setClosing(false);
        setState(io::ChannelState::Closed);
    }

    FakeIoDevice& fakeDevice() { return dev_; }
    int channelErrorHookCount() const { return channelErrorHookCount_; }

    void triggerBytesFlushedQueued(qint64 bytes) {
        QMetaObject::invokeMethod(&dev_, [this, bytes]() {
            dev_.simulateBytesFlushed(bytes);
        }, Qt::QueuedConnection);
    }

protected:
    QIODevice* device() override { return &dev_; }
    bool isWritable() const override {
        return state() == io::ChannelState::Open && dev_.isOpen();
    }
    QString logContext() const override { return QStringLiteral("fake"); }
    QString errorPrefix() const override { return QStringLiteral("Fake"); }
    void onChannelError() override { ++channelErrorHookCount_; }

private:
    FakeIoDevice dev_;
    int channelErrorHookCount_ = 0;
};

class BufferedSignalRelay : public QObject {
    Q_OBJECT
public:
    void attach(TestBufferedChannel& channel) {
        stateId_ = channel.addStateHandler([this](io::ChannelState s) {
            emit stateChanged(s);
        });
        channel.setErrorHandler([this](const io::ChannelError& err) {
            emit errorOccurred(err.code, err.message);
        });
        channel.setMonitor([this](bool isTx, const QByteArray& data) {
            emit monitored(isTx, data);
        });
        channel.setWriteDrainedHandler([this]() {
            emit writeDrained();
        });
    }

    void detach(TestBufferedChannel& channel) {
        channel.removeStateHandler(stateId_);
        stateId_ = 0;
        channel.setErrorHandler(nullptr);
        channel.setMonitor(nullptr);
        channel.setWriteDrainedHandler(nullptr);
    }

signals:
    void stateChanged(io::ChannelState s);
    void errorOccurred(io::ChannelErrorCode code, const QString& msg);
    void monitored(bool isTx, const QByteArray& data);
    void writeDrained();

private:
    io::IChannel::HandlerId stateId_ = 0;
};

bool waitForCondition(QSignalSpy& spy, const std::function<bool()>& pred, int timeoutMs = 2000) {
    if (pred()) {
        return true;
    }
    QElapsedTimer timer;
    timer.start();
    while (!pred()) {
        const int remaining = timeoutMs - static_cast<int>(timer.elapsed());
        if (remaining <= 0) {
            return false;
        }
        spy.wait(remaining);
    }
    return true;
}

TEST(BufferedWritingChannelTest, MultiBlockEnqueue_DoesNotStallBehindUnflushedBytes) {
    TestBufferedChannel channel;
    BufferedSignalRelay relay;
    relay.attach(channel);
    QSignalSpy drainedSpy(&relay, &BufferedSignalRelay::writeDrained);

    ASSERT_TRUE(channel.write(QByteArrayLiteral("AAAA")));
    ASSERT_TRUE(channel.write(QByteArrayLiteral("BBBB")));
    ASSERT_TRUE(channel.write(QByteArrayLiteral("CCCC")));

    // All three blocks must be pushed to QIODevice immediately rather than
    // stalling blocks 2 and 3 until block 1 finishes draining.
    EXPECT_EQ(channel.fakeDevice().writeCallCount(), 3);
    EXPECT_EQ(channel.fakeDevice().writtenData(), QByteArrayLiteral("AAAABBBBCCCC"));
    EXPECT_EQ(drainedSpy.count(), 0);

    channel.triggerBytesFlushedQueued(4);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_EQ(drainedSpy.count(), 0);
    EXPECT_EQ(channel.stats().bytesTx, 4u);

    channel.triggerBytesFlushedQueued(8);
    ASSERT_TRUE(waitForCondition(drainedSpy, [&drainedSpy]() {
        return drainedSpy.count() >= 1;
    }));
    EXPECT_EQ(drainedSpy.count(), 1);
    EXPECT_EQ(channel.stats().bytesTx, 12u);

    relay.detach(channel);
}

TEST(BufferedWritingChannelTest, PartialWrite_PreservesFrameOrderAndCompletesOnDrain) {
    TestBufferedChannel channel;
    channel.fakeDevice().setMaxBufferedBytes(3);
    BufferedSignalRelay relay;
    relay.attach(channel);
    QSignalSpy drainedSpy(&relay, &BufferedSignalRelay::writeDrained);

    ASSERT_TRUE(channel.write(QByteArrayLiteral("ABCDEF")));
    ASSERT_TRUE(channel.write(QByteArrayLiteral("GH")));

    // Must stop after partial write of frame 1 ("ABC") without overtaking with "GH".
    EXPECT_EQ(channel.fakeDevice().writtenData(), QByteArrayLiteral("ABC"));

    channel.triggerBytesFlushedQueued(3);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_EQ(channel.fakeDevice().writtenData(), QByteArrayLiteral("ABCDEF"));

    channel.triggerBytesFlushedQueued(3);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_EQ(channel.fakeDevice().writtenData(), QByteArrayLiteral("ABCDEFGH"));

    channel.triggerBytesFlushedQueued(2);
    ASSERT_TRUE(waitForCondition(drainedSpy, [&drainedSpy]() {
        return drainedSpy.count() >= 1;
    }));
    EXPECT_EQ(channel.stats().bytesTx, 8u);

    relay.detach(channel);
}

TEST(BufferedWritingChannelTest, WriteTimeout_NotResetByRepeatedWriteCalls) {
    TestBufferedChannel channel;
    io::Timeouts timeouts;
    timeouts.writeMs = 80;
    channel.setTimeouts(timeouts);

    BufferedSignalRelay relay;
    relay.attach(channel);
    QSignalSpy errorSpy(&relay, &BufferedSignalRelay::errorOccurred);

    ASSERT_TRUE(channel.write(QByteArrayLiteral("first")));

    // Repeated writes every 25ms for 250ms must not starve/reset the 80ms write timeout.
    QTimer periodicWriter;
    QObject::connect(&periodicWriter, &QTimer::timeout, &periodicWriter, [&]() {
        if (channel.state() == io::ChannelState::Open) {
            channel.write(QByteArrayLiteral("keep"));
        }
    });
    periodicWriter.start(25);

    ASSERT_TRUE(waitForCondition(errorSpy, [&errorSpy]() {
        return errorSpy.count() >= 1;
    }, 180));
    periodicWriter.stop();

    EXPECT_EQ(errorSpy.takeFirst().at(0).value<io::ChannelErrorCode>(),
              io::ChannelErrorCode::Timeout);
    EXPECT_EQ(channel.state(), io::ChannelState::Error);
    EXPECT_EQ(channel.channelErrorHookCount(), 1);

    relay.detach(channel);
}

TEST(BufferedWritingChannelTest, SyncWriteFailure_ReturnsFalseSuppressesMonitorAndGuardsLateBytesWritten) {
    TestBufferedChannel channel;
    channel.fakeDevice().setFailNextWrite(true);

    BufferedSignalRelay relay;
    relay.attach(channel);
    QSignalSpy errorSpy(&relay, &BufferedSignalRelay::errorOccurred);
    QSignalSpy monitorSpy(&relay, &BufferedSignalRelay::monitored);
    QSignalSpy drainedSpy(&relay, &BufferedSignalRelay::writeDrained);

    EXPECT_FALSE(channel.write(QByteArrayLiteral("frame")));
    EXPECT_EQ(channel.state(), io::ChannelState::Error);
    EXPECT_EQ(channel.channelErrorHookCount(), 1);
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).value<io::ChannelErrorCode>(),
              io::ChannelErrorCode::WriteFailed);
    EXPECT_EQ(monitorSpy.count(), 0);

    // Late bytesWritten after Error must be ignored safely.
    channel.triggerBytesFlushedQueued(5);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    EXPECT_EQ(drainedSpy.count(), 0);
    EXPECT_EQ(channel.stats().bytesTx, 0u);

    relay.detach(channel);
}

} // namespace

#include "BufferedWritingChannelTest.moc"
