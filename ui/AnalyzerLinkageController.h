#pragma once

#include <QObject>
#include <optional>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"

namespace ui::views::modbus_tcp {
class ModbusTcpView;
}

namespace ui::views::modbus_rtu {
class ModbusRtuView;
}

namespace ui::widgets {
class FrameAnalyzerWidget;
}

namespace ui {

class AnalyzerLinkageController : public QObject {
    Q_OBJECT

public:
    enum class LinkSource {
        None,
        Tcp,
        Rtu
    };

    enum class LinkState {
        Idle,
        Live,
        Paused
    };

    struct State {
        LinkState state = LinkState::Idle;
        LinkSource source = LinkSource::None;
    };

    explicit AnalyzerLinkageController(QObject* parent = nullptr);

    void bind(views::modbus_tcp::ModbusTcpView* tcpView,
              views::modbus_rtu::ModbusRtuView* rtuView,
              widgets::FrameAnalyzerWidget* frameAnalyzer);

    State state() const;

    void requestLinkToggle(LinkSource source, bool active);
    void requestStop();
    void requestPause(bool paused);
    void handleLiveData(const modbus::base::Pdu& pdu,
                        modbus::core::parser::ProtocolType protocol,
                        uint16_t addr);
    void handleSourceDisconnected(LinkSource source);

    static LinkSource sourceFromProtocol(modbus::core::parser::ProtocolType protocol);

private:
    struct BufferedLiveData {
        modbus::base::Pdu pdu;
        modbus::core::parser::ProtocolType protocol = modbus::core::parser::ProtocolType::Tcp;
        uint16_t addr = 0;
    };

    void transitionTo(LinkState state, LinkSource source);
    void applyState(const State& previousState);
    void clearBufferedLiveData();
    void replayBufferedLiveDataIfNeeded();
    void stopInternal();

    State state_;
    std::optional<BufferedLiveData> bufferedLiveData_;
    views::modbus_tcp::ModbusTcpView* tcpView_ = nullptr;
    views::modbus_rtu::ModbusRtuView* rtuView_ = nullptr;
    widgets::FrameAnalyzerWidget* frameAnalyzer_ = nullptr;
};

} // namespace ui
