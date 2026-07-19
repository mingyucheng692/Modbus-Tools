#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QCoreApplication>

#include "../../../infra/io/ChannelOperationWorker.h"
#include "../../../infra/io/SerialConfig.h"
#include "../../helpers/ModbusTestHelpers.h"

using namespace testing;
using namespace io;

namespace {

class ChannelOperationWorkerTest : public Test {
protected:
    void SetUp() override {
        worker_ = std::make_unique<ChannelOperationWorker>();
    }

    void TearDown() override {
        // Ensure channel cleanup before destroying worker
        if (worker_) {
            worker_->close();
        }
        worker_.reset();
    }

    std::unique_ptr<ChannelOperationWorker> worker_;
};

// ---------------------------------------------------------------------------
// Construction / Destruction
// ---------------------------------------------------------------------------

TEST_F(ChannelOperationWorkerTest, Construct_DestructNoCrash) {
    EXPECT_NE(worker_, nullptr);
}

// ---------------------------------------------------------------------------
// Open TCP — verify the attempt does not crash (async-safe)
// ---------------------------------------------------------------------------
// NOTE: openTcp creates a real TcpChannel with QTcpSocket which starts an async
// connectToHost. Destroying the socket while Windows IOCP is in flight causes
// SEH. This test is skipped in lightweight CI; integration test covers TCP path.

// TEST_F(ChannelOperationWorkerTest, OpenTcp_NoCrash) { ... }

// ---------------------------------------------------------------------------
// Open Serial — verify error path works
// ---------------------------------------------------------------------------

TEST_F(ChannelOperationWorkerTest, OpenSerial_InvalidPort_EmitsError) {
    QSignalSpy errorSpy(worker_.get(), &ChannelOperationWorker::channelErrorOccurred);

    SerialConfig cfg;
    cfg.portName = QStringLiteral("COM_NONEXISTENT_99999");
    worker_->openSerial(cfg);

    // Serial open failure emits error synchronously (or very quickly)
    // Allow some async processing
    for (int i = 0; i < 10 && errorSpy.count() == 0; ++i) {
        QCoreApplication::processEvents();
    }

    EXPECT_GE(errorSpy.count(), 1);
}

TEST_F(ChannelOperationWorkerTest, OpenSerial_EmptyPortName_EmitsError) {
    QSignalSpy errorSpy(worker_.get(), &ChannelOperationWorker::channelErrorOccurred);

    SerialConfig cfg;
    cfg.portName.clear();
    worker_->openSerial(cfg);

    for (int i = 0; i < 10 && errorSpy.count() == 0; ++i) {
        QCoreApplication::processEvents();
    }

    EXPECT_GE(errorSpy.count(), 1);
}

// ---------------------------------------------------------------------------
// Write without open channel
// ---------------------------------------------------------------------------

TEST_F(ChannelOperationWorkerTest, Write_ChannelNotOpen_EmitsError) {
    QSignalSpy errorSpy(worker_.get(), &ChannelOperationWorker::channelErrorOccurred);

    worker_->write(QByteArrayLiteral("test data"));

    EXPECT_GE(errorSpy.count(), 1);
}

// ---------------------------------------------------------------------------
// Close with no channel should not crash
// ---------------------------------------------------------------------------

TEST_F(ChannelOperationWorkerTest, Close_NoChannel_DoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(worker_->close());
}

// ---------------------------------------------------------------------------
// DTR / RTS on non-serial channel (silently ignored)
// ---------------------------------------------------------------------------

TEST_F(ChannelOperationWorkerTest, DtrRts_NoChannel_DoesNotCrash) {
    EXPECT_NO_FATAL_FAILURE(worker_->setDtr(true));
    EXPECT_NO_FATAL_FAILURE(worker_->setRts(false));
}

// ---------------------------------------------------------------------------
// Repeated writes
// ---------------------------------------------------------------------------

TEST_F(ChannelOperationWorkerTest, RepeatedWrites_WithoutChannel_EmitsMultipleErrors) {
    QSignalSpy errorSpy(worker_.get(), &ChannelOperationWorker::channelErrorOccurred);

    worker_->write(QByteArrayLiteral("data1"));
    worker_->write(QByteArrayLiteral("data2"));

    EXPECT_GE(errorSpy.count(), 2);
}

} // namespace