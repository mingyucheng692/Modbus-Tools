#pragma once

#include <QWidget>
#include <memory>
#include "../../../core/io/SerialChannel.h"

class QVBoxLayout;
class QLabel;

namespace modbus::dispatch { class ModbusWorker; }
namespace modbus::session { class IModbusClient; }

namespace ui::widgets {
    class SerialConnectionWidget;
    class FunctionWidget;
    class TrafficMonitorWidget;
}

namespace ui::views::rtu {

class RtuView : public QWidget {
    Q_OBJECT

public:
    explicit RtuView(QWidget *parent = nullptr);
    ~RtuView() override;

private:
    void setupUi();

    QVBoxLayout* mainLayout_ = nullptr;
    ui::widgets::SerialConnectionWidget* connectionWidget_ = nullptr;
    ui::widgets::FunctionWidget* functionWidget_ = nullptr;
    ui::widgets::TrafficMonitorWidget* trafficMonitor_ = nullptr;

    std::shared_ptr<io::SerialChannel> channel_;
    std::shared_ptr<modbus::session::IModbusClient> client_;
    std::shared_ptr<modbus::dispatch::ModbusWorker> worker_;
};

} // namespace ui::views::rtu
