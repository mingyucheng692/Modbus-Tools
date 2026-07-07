#pragma once

#include <QObject>
#include <optional>
#include <cstdint>

#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "../application/modbus/ModbusTypes.h"

namespace ui::views::modbus { class ModbusPage; }
namespace ui::widgets { class FrameAnalyzerWidget; }

namespace ui::application {

/**
 * @brief Owns the live-linkage state machine and wires the Modbus page and
 *        Frame Analyzer signals to it.
 *
 * Single entry point for all live-linkage operations. With T9, the three
 * former Modbus pages (TCP/RTU/ASCII) are merged into one ModbusPage that
 * supports in-page protocol switching. The active link source is determined
 * by querying the page's currentMode() at signal delivery time.
 *
 * T10 will further simplify this coordinator (single-source model).
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

    void bind(views::modbus::ModbusPage* modbusView,
              widgets::FrameAnalyzerWidget* frameAnalyzer);

    State state() const;

private:
    // --- Signal handlers (wired in bind()) ---
    void handleLinkageToggled(bool active);
    void handleLinkageStopRequested();
    void handleLinkagePauseToggled(bool paused);
    void handleLinkageData(const ::modbus::base::Pdu& pdu,
                           ::modbus::core::parser::ProtocolType protocol,
                           uint16_t addr);
    void handleSourceDisconnected();

    // --- State machine core ---
    void requestLinkToggle(LinkSource source, bool active);
    void requestStop();
    void requestPause(bool paused);
    void handleLiveData(const ::modbus::base::Pdu& pdu,
                        ::modbus::core::parser::ProtocolType protocol,
                        uint16_t addr);
    void handleSourceDisconnected(LinkSource source);
    static LinkSource sourceFromProtocol(::modbus::core::parser::ProtocolType protocol);
    static LinkSource sourceFromMode(::ui::application::modbus::SessionMode mode);

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
    views::modbus::ModbusPage* modbusView_ = nullptr;
    widgets::FrameAnalyzerWidget* frameAnalyzer_ = nullptr;
};

} // namespace ui::application
