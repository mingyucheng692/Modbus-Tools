#pragma once

#include <QObject>
#include <optional>
#include <cstdint>

#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"

namespace ui::views::modbus { class ModbusPage; }
namespace ui::widgets { class FrameAnalyzerWidget; }

namespace ui::application {

/**
 * @brief Owns the live-linkage state machine and wires the single Modbus page
 *        and Frame Analyzer signals to it.
 *
 * Single-source model (T10): with the three former Modbus pages merged into
 * one ModbusPage (T9), there is exactly one possible link source. The state
 * machine tracks only Idle/Live/Paused — no source dimension.
 */
class AnalyzerLinkCoordinator final : public QObject {
    Q_OBJECT

public:
    enum class LinkState {
        Idle,
        Live,
        Paused
    };

    explicit AnalyzerLinkCoordinator(QObject* parent = nullptr);
    ~AnalyzerLinkCoordinator() noexcept override;

    void bind(views::modbus::ModbusPage* modbusView,
              widgets::FrameAnalyzerWidget* frameAnalyzer);

    LinkState state() const;

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
    void requestLinkToggle(bool active);
    void requestStop();
    void requestPause(bool paused);
    void handleLiveData(const ::modbus::base::Pdu& pdu,
                        ::modbus::core::parser::ProtocolType protocol,
                        uint16_t addr);
    void transitionTo(LinkState state);
    void applyState();
    void clearBufferedLiveData();
    void replayBufferedLiveDataIfNeeded();
    void stopInternal();

    struct BufferedLiveData {
        ::modbus::base::Pdu pdu;
        ::modbus::core::parser::ProtocolType protocol = ::modbus::core::parser::ProtocolType::Tcp;
        uint16_t addr = 0;
    };

    LinkState state_ = LinkState::Idle;
    std::optional<BufferedLiveData> bufferedLiveData_;
    views::modbus::ModbusPage* modbusView_ = nullptr;
    widgets::FrameAnalyzerWidget* frameAnalyzer_ = nullptr;
};

} // namespace ui::application
