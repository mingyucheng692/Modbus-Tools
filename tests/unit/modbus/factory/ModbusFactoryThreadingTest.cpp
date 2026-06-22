#include <gtest/gtest.h>

#include "modbus/factory/ModbusFactory.h"
#include "helpers/ModbusTestHelpers.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QPointer>

using namespace modbus::factory;
using namespace modbus::base;

namespace {

bool waitForCondition(const std::function<bool()>& predicate, int timeoutMs = 1000)
{
    QElapsedTimer timer;
    timer.start();
    while (!predicate() && timer.elapsed() < timeoutMs) {
        QCoreApplication::processEvents();
    }
    return predicate();
}

} // namespace

TEST(ModbusFactoryThreadingTest, CreateTcpStack_UsesDedicatedIoAndWorkerThreads) {
    ModbusFactory factory;
    auto config = modbus::test::MakeModbusConfig(ModbusMode::TCP);

    ModbusStack stack = factory.createStack(config);

    ASSERT_TRUE(stack.channel);
    ASSERT_TRUE(stack.client);
    ASSERT_TRUE(stack.worker);
    ASSERT_TRUE(stack.ioThread);
    ASSERT_TRUE(stack.thread);
    EXPECT_NE(stack.ioThread.get(), stack.thread.get());
}

TEST(ModbusFactoryThreadingTest, DestroyUnstartedStack_DeletesWorkerAndThreadsSynchronously) {
    QPointer<QThread> ioThread;
    QPointer<QThread> workerThread;
    QPointer<modbus::dispatch::ModbusWorker> worker;

    {
        ModbusFactory factory;
        auto config = modbus::test::MakeModbusConfig(ModbusMode::TCP);
        ModbusStack stack = factory.createStack(config);

        ASSERT_TRUE(stack.ioThread);
        ASSERT_TRUE(stack.thread);
        ASSERT_TRUE(stack.worker);

        ioThread = stack.ioThread.get();
        workerThread = stack.thread.get();
        worker = stack.worker.get();
    }

    EXPECT_TRUE(ioThread.isNull());
    EXPECT_TRUE(workerThread.isNull());
    EXPECT_TRUE(worker.isNull());
}

TEST(ModbusFactoryThreadingTest, DestroyStoppedStartedStack_ReleasesWorkerAndThreads) {
    ModbusFactory factory;
    auto config = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    ModbusStack stack = factory.createStack(config);

    ASSERT_TRUE(stack.ioThread);
    ASSERT_TRUE(stack.thread);
    ASSERT_TRUE(stack.worker);

    QPointer<QThread> ioThread = stack.ioThread.get();
    QPointer<QThread> workerThread = stack.thread.get();
    QPointer<modbus::dispatch::ModbusWorker> worker = stack.worker.get();

    stack.ioThread->start();
    stack.worker->start();

    ASSERT_TRUE(waitForCondition([&]() {
        return stack.ioThread->isRunning() && stack.thread->isRunning();
    }));

    stack.worker->stop();
    stack.thread->quit();
    stack.ioThread->quit();

    ASSERT_TRUE(stack.thread->wait(1000));
    ASSERT_TRUE(stack.ioThread->wait(1000));

    stack = ModbusStack{};

    EXPECT_TRUE(ioThread.isNull());
    EXPECT_TRUE(workerThread.isNull());
    EXPECT_TRUE(worker.isNull());
}

TEST(ModbusFactoryThreadingTest, CreateRtuStack_UsesDedicatedIoAndWorkerThreads) {
    ModbusFactory factory;
    auto config = modbus::test::MakeModbusConfig(ModbusMode::RTU);
    config.portName = "COM1";

    ModbusStack stack = factory.createStack(config);

    ASSERT_TRUE(stack.channel);
    ASSERT_TRUE(stack.client);
    ASSERT_TRUE(stack.worker);
    ASSERT_TRUE(stack.ioThread);
    ASSERT_TRUE(stack.thread);
    EXPECT_NE(stack.ioThread.get(), stack.thread.get());
}
