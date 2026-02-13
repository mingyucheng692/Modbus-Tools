#pragma once

#include <QWidget>
#include <memory>

class QVBoxLayout;
class QLabel;

namespace modbus::dispatch { class ModbusWorker; }
namespace modbus::session { class IModbusClient; }
namespace io { class TcpChannel; }

namespace ui::widgets {
    class TcpConnectionWidget;
    class FunctionWidget;
    class TrafficMonitorWidget;
}

namespace ui::views::tcp {

class TcpView : public QWidget {
    Q_OBJECT

public:
    explicit TcpView(QWidget *parent = nullptr);
    ~TcpView() override;

private:
    void setupUi();

    QVBoxLayout* mainLayout_ = nullptr;
    ui::widgets::TcpConnectionWidget* connectionWidget_ = nullptr;
    ui::widgets::FunctionWidget* functionWidget_ = nullptr;
    ui::widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;

    std::shared_ptr<io::TcpChannel> channel_;
    std::shared_ptr<modbus::session::IModbusClient> client_;
    std::shared_ptr<modbus::dispatch::ModbusWorker> worker_;
};

} // namespace ui::views::tcp
