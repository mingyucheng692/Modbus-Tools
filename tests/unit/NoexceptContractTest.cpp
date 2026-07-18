#include <gtest/gtest.h>

#include "../../infra/io/ChannelOperationWorker.h"
#include "../../infra/io/SerialChannel.h"
#include "../../infra/io/SerialConfig.h"
#include "../../infra/io/ServerChannelWorker.h"
#include "../../infra/io/TcpChannel.h"
#include "../../infra/io/TcpServerHandle.h"
#include "../../infra/io/UdpChannel.h"
#include "../../core/modbus/dispatch/ModbusWorker.h"
#include "../../core/modbus/parser/FrameParseWorker.h"
#include "../../core/modbus/session/SessionTypes.h"
#include "../../core/modbus/session/ModbusClient.h"
#include "../../core/modbus/transport/ITransport.h"
#include "../../core/update/UpdateManager.h"
#include "../../ui/application/AnalyzerLinkCoordinator.h"
#include "../../ui/application/modbus/ModbusSessionPresenter.h"
#include "../../ui/widgets/LogRecorder.h"

#include <type_traits>

static_assert(std::is_nothrow_destructible_v<ui::widgets::LogRecorder>);
static_assert(std::is_nothrow_destructible_v<modbus::parser::FrameParseWorker>);
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
static_assert(std::is_nothrow_destructible_v<modbus::transport::ITransport>);

TEST(NoexceptContract, PriorityClassesRemainNothrowDestructible)
{
    EXPECT_TRUE((std::is_nothrow_destructible_v<ui::widgets::LogRecorder>));
    EXPECT_TRUE((std::is_nothrow_destructible_v<modbus::parser::FrameParseWorker>));
    EXPECT_TRUE((std::is_nothrow_destructible_v<core::update::UpdateManager>));
    EXPECT_TRUE((std::is_nothrow_destructible_v<modbus::session::ModbusClient>));
    EXPECT_TRUE((std::is_nothrow_destructible_v<ui::application::modbus::ModbusSessionPresenter>));
}
