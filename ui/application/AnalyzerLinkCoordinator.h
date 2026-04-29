#pragma once

#include <QObject>
#include <memory>
#include <cstdint>

#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"

namespace ui {
class AnalyzerLinkageController;
namespace views::modbus_tcp { class ModbusTcpView; }
namespace views::modbus_rtu { class ModbusRtuView; }
namespace widgets { class FrameAnalyzerWidget; }
}

namespace ui::application {

class AnalyzerLinkCoordinator final : public QObject {
    Q_OBJECT

public:
    explicit AnalyzerLinkCoordinator(QObject* parent = nullptr);
    ~AnalyzerLinkCoordinator() override;

    void bind(views::modbus_tcp::ModbusTcpView* tcpView,
              views::modbus_rtu::ModbusRtuView* rtuView,
              widgets::FrameAnalyzerWidget* frameAnalyzer);

private:
    void handleTcpLinkageToggled(bool active);
    void handleRtuLinkageToggled(bool active);
    void handleLinkageStopRequested();
    void handleLinkagePauseToggled(bool paused);
    void handleLinkageData(const modbus::base::Pdu& pdu,
                           modbus::core::parser::ProtocolType protocol,
                           uint16_t addr);
    void handleTcpSourceDisconnected();
    void handleRtuSourceDisconnected();

    std::unique_ptr<AnalyzerLinkageController> controller_;
};

} // namespace ui::application
