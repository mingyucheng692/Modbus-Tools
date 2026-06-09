#include <gtest/gtest.h>

#include "../../core/io/ChannelOperationWorker.h"
#include "../../core/io/SerialChannel.h"
#include "../../core/io/SerialConfig.h"
#include "../../core/io/ServerChannelWorker.h"
#include "../../core/io/TcpChannel.h"
#include "../../core/io/TcpServerHandle.h"
#include "../../core/io/UdpChannel.h"
#include "../../core/modbus/dispatch/ModbusWorker.h"
#include "../../core/modbus/factory/IModbusFactory.h"
#include "../../core/modbus/parser/FrameParseWorker.h"
#include "../../core/modbus/session/IModbusClient.h"
#include "../../core/modbus/session/ModbusClient.h"
#include "../../core/modbus/transport/ITransport.h"
#include "../../core/update/UpdateManager.h"
#include "../../ui/application/AnalyzerLinkCoordinator.h"
#include "../../ui/application/modbus/ModbusSessionPresenter.h"
#include "../../ui/widgets/LogRecorder.h"

#include <type_traits>

static_assert(std::is_nothrow_destructible_v<ui::widgets::LogRecorder>);
static_assert(std::is_nothrow_destructible_v<modbus::core::parser::FrameParseWorker>);
static_assert(std::is_nothrow_destructible_v<core::update::UpdateManager>);
static_assert(std::is_nothrow_destructible_v<modbus::session::ModbusClient>);
static_assert(std::is_nothrow_destructible_v<modbus::dispatch::ModbusWorker>);
static_assert(std::is_nothrow_destructible_v<io::ChannelOperationWorker>);
static_assert(std::is_nothrow_destructible_v<io::ServerChannelWorker>);
static_assert(std::is_nothrow_destructible_v<io::SerialChannel>);
static_assert(std::is_nothrow_destructible_v<io::TcpChannel>);
static_assert(std::is_nothrow_destructible_v<io::UdpChannel>);
static_assert(std::is_nothrow_destructible_v<io::TcpServerHandle>);
static_assert(std::is_nothrow_destructible_v<ui::application::AnalyzerLinkCoordinator>);
static_assert(std::is_nothrow_destructible_v<ui::application::modbus::ModbusSessionPresenter>);
static_assert(std::is_nothrow_destructible_v<io::IChannel>);
static_assert(std::is_nothrow_destructible_v<modbus::session::IModbusClient>);
static_assert(std::is_nothrow_destructible_v<modbus::transport::ITransport>);
static_assert(std::is_nothrow_destructible_v<modbus::factory::IModbusFactory>);

TEST(NoexceptContract, PriorityClassesRemainNothrowDestructible)
{
    EXPECT_TRUE((std::is_nothrow_destructible_v<ui::widgets::LogRecorder>));
    EXPECT_TRUE((std::is_nothrow_destructible_v<modbus::core::parser::FrameParseWorker>));
    EXPECT_TRUE((std::is_nothrow_destructible_v<core::update::UpdateManager>));
    EXPECT_TRUE((std::is_nothrow_destructible_v<modbus::session::ModbusClient>));
    EXPECT_TRUE((std::is_nothrow_destructible_v<ui::application::modbus::ModbusSessionPresenter>));
}
