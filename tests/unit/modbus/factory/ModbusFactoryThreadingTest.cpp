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
    auto config = modbus::test::MakeModbusConfig(ModbusMode::TCP);

    auto stackOpt = createStack(config);
    ASSERT_TRUE(stackOpt);
    ModbusStack stack = std::move(*stackOpt);

    ASSERT_TRUE(stack.channel);
    ASSERT_TRUE(stack.client);
    ASSERT_TRUE(stack.worker);
    ASSERT_TRUE(stack.thread);
}

TEST(ModbusFactoryThreadingTest, DestroyUnstartedStack_DeletesWorkerAndThreadsSynchronously) {
    QPointer<QThread> workerThread;
    QPointer<modbus::dispatch::ModbusWorker> worker;

    {
        auto config = modbus::test::MakeModbusConfig(ModbusMode::TCP);
        auto stackOpt = createStack(config);
        ASSERT_TRUE(stackOpt);
        ModbusStack stack = std::move(*stackOpt);

        ASSERT_TRUE(stack.thread);
        ASSERT_TRUE(stack.worker);

        workerThread = stack.thread.get();
        worker = stack.worker.get();
    }

    EXPECT_TRUE(workerThread.isNull());
    EXPECT_TRUE(worker.isNull());
}

TEST(ModbusFactoryThreadingTest, DestroyStoppedStartedStack_ReleasesWorkerAndThreads) {
    auto config = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    auto stackOpt = createStack(config);
    ASSERT_TRUE(stackOpt);
    ModbusStack stack = std::move(*stackOpt);

    ASSERT_TRUE(stack.thread);
    ASSERT_TRUE(stack.worker);

    QPointer<QThread> workerThread = stack.thread.get();
    QPointer<modbus::dispatch::ModbusWorker> worker = stack.worker.get();

    stack.thread->start();
    stack.worker->start();

    ASSERT_TRUE(waitForCondition([&]() {
        return stack.thread->isRunning();
    }));

    stack.worker->stop();
    stack.thread->quit();

    ASSERT_TRUE(stack.thread->wait(1000));

    stack = ModbusStack{};

    EXPECT_TRUE(workerThread.isNull());
    EXPECT_TRUE(worker.isNull());
}

TEST(ModbusFactoryThreadingTest, ReleasingStartedStack_ShutsDownWorkerAndIoThreads) {
    auto config = modbus::test::MakeModbusConfig(ModbusMode::TCP);
    auto stackOpt = createStack(config);
    ASSERT_TRUE(stackOpt);
    ModbusStack stack = std::move(*stackOpt);

    ASSERT_TRUE(stack.thread);
    ASSERT_TRUE(stack.worker);

    QPointer<QThread> workerThread = stack.thread.get();
    QPointer<modbus::dispatch::ModbusWorker> worker = stack.worker.get();

    stack.thread->start();
    stack.worker->start();

    ASSERT_TRUE(waitForCondition([&]() {
        return workerThread && workerThread->isRunning();
    }));

    stack = ModbusStack{};

    ASSERT_TRUE(waitForCondition([&]() { return worker.isNull(); }));
    ASSERT_TRUE(waitForCondition([&]() { return workerThread.isNull(); }));
}

TEST(ModbusFactoryThreadingTest, CreateRtuStack_UsesDedicatedIoAndWorkerThreads) {
    auto config = modbus::test::MakeModbusConfig(ModbusMode::RTU);
    config.portName = "COM1";

    auto stackOpt = createStack(config);
    ASSERT_TRUE(stackOpt);
    ModbusStack stack = std::move(*stackOpt);

    ASSERT_TRUE(stack.channel);
    ASSERT_TRUE(stack.client);
    ASSERT_TRUE(stack.worker);
    ASSERT_TRUE(stack.thread);
}
