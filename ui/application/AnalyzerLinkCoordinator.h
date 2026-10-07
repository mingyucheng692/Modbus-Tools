// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <optional>
#include <cstdint>

#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"

namespace ui::views::modbus { class ModbusPage; }
namespace ui::views::analyzer { class FrameAnalyzerView; }

namespace ui::application {

// Role: Coordinates live frame forwarding between ModbusPage and FrameAnalyzerView (Idle/Live/Paused).
// Thread-safety: Single-threaded (GUI thread only).
// Ownership: Owned by application presentation coordinator via QObject parent tree.
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
              views::analyzer::FrameAnalyzerView* frameAnalyzer);

    LinkState state() const;

private:
    // --- Signal handlers (wired in bind()) ---
    void handleLinkageToggled(bool active);
    void handleLinkageStopRequested();
    void handleLinkagePauseToggled(bool paused);
    void handleLinkageData(const ::modbus::base::Pdu& pdu,
                           ::modbus::parser::ProtocolType protocol,
                           uint16_t addr);
    void handleSourceDisconnected();

    // --- State machine core ---
    void requestLinkToggle(bool active);
    void requestStop();
    void requestPause(bool paused);
    void handleLiveData(const ::modbus::base::Pdu& pdu,
                        ::modbus::parser::ProtocolType protocol,
                        uint16_t addr);
    void transitionTo(LinkState state);
    void applyState();
    void clearBufferedLiveData();
    void replayBufferedLiveDataIfNeeded();
    void stopInternal();

    struct BufferedLiveData {
        ::modbus::base::Pdu pdu;
        ::modbus::parser::ProtocolType protocol = ::modbus::parser::ProtocolType::Tcp;
        uint16_t addr = 0;
    };

    LinkState state_ = LinkState::Idle;
    std::optional<BufferedLiveData> bufferedLiveData_;
    views::modbus::ModbusPage* modbusView_ = nullptr;
    views::analyzer::FrameAnalyzerView* frameAnalyzer_ = nullptr;
};

} // namespace ui::application
