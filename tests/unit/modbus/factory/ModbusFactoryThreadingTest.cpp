#include <gtest/gtest.h>

#include "modbus/factory/ModbusFactory.h"
#include "helpers/ModbusTestHelpers.h"

using namespace modbus::factory;
using namespace modbus::base;

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
