#pragma once

#include <QObject>
#include <optional>
#include <cstdint>

#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"

namespace ui::views { class BaseModbusPage; }
namespace ui::widgets { class FrameAnalyzerWidget; }

namespace ui::application {

/**
 * @brief Owns the live-linkage state machine and wires TCP/RTU page and Frame
 *        Analyzer signals to it.
 *
 * Single entry point for all live-linkage operations. The former
 * AnalyzerLinkageController pass-through layer has been merged in: this class
 * now holds the Idle/Live/Paused × None/Tcp/Rtu state machine directly and
 * connects the view signals in bind().
 */
class AnalyzerLinkCoordinator final : public QObject {
    Q_OBJECT

public:
    enum class LinkSource {
        None,
        Tcp,
        Rtu,
        Ascii
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

    explicit AnalyzerLinkCoordinator(QObject* parent = nullptr);
    ~AnalyzerLinkCoordinator() noexcept override;

    void bind(views::BaseModbusPage* tcpView,
              views::BaseModbusPage* rtuView,
              views::BaseModbusPage* asciiView,
              widgets::FrameAnalyzerWidget* frameAnalyzer);

    State state() const;

private:
    // --- Signal handlers (wired in bind()) ---
    void handleTcpLinkageToggled(bool active);
    void handleRtuLinkageToggled(bool active);
    void handleLinkageStopRequested();
    void handleLinkagePauseToggled(bool paused);
    void handleLinkageData(const ::modbus::base::Pdu& pdu,
                           ::modbus::core::parser::ProtocolType protocol,
                           uint16_t addr);
    void handleTcpSourceDisconnected();
    void handleRtuSourceDisconnected();

    // --- State machine core ---
    void requestLinkToggle(LinkSource source, bool active);
    void requestStop();
    void requestPause(bool paused);
    void handleLiveData(const ::modbus::base::Pdu& pdu,
                        ::modbus::core::parser::ProtocolType protocol,
                        uint16_t addr);
    void handleSourceDisconnected(LinkSource source);
    static LinkSource sourceFromProtocol(::modbus::core::parser::ProtocolType protocol);

    void transitionTo(LinkState state, LinkSource source);
    void applyState(const State& previousState);
    void clearBufferedLiveData();
    void replayBufferedLiveDataIfNeeded();
    void stopInternal();

    struct BufferedLiveData {
        ::modbus::base::Pdu pdu;
        ::modbus::core::parser::ProtocolType protocol = ::modbus::core::parser::ProtocolType::Tcp;
        uint16_t addr = 0;
    };

    State state_;
    std::optional<BufferedLiveData> bufferedLiveData_;
    views::BaseModbusPage* tcpView_ = nullptr;
    views::BaseModbusPage* rtuView_ = nullptr;
    views::BaseModbusPage* asciiView_ = nullptr;
    widgets::FrameAnalyzerWidget* frameAnalyzer_ = nullptr;
};

} // namespace ui::application
