// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QThread>
#include <memory>
#include "modbus/base/ModbusTypes.h"
#include "modbus/parser/ModbusFrameParser.h"

namespace modbus::parser {
class FrameParseWorker;
}

namespace ui::application::analyzer {

// Role: Owns the background QThread and FrameParseWorker for logical frame parsing.
// Thread-safety: Single-threaded (bound to GUI thread); cross-thread worker calls queued.
// Ownership: Owned by QObject parent (FrameAnalyzerView); manages background worker and QThread.
class FrameAnalyzerPresenter : public QObject {
    Q_OBJECT
public:
    explicit FrameAnalyzerPresenter(QObject* parent = nullptr);
    ~FrameAnalyzerPresenter() override;

    // Queues a parse request onto the worker thread; input must be pre-normalized hex string.
    void enqueueParse(const QString& input,
                      modbus::parser::ProtocolType type,
                      uint16_t startAddress,
                      modbus::base::RegisterOrder order,
                      quint64 requestId);

signals:
    /// Relay of FrameParseWorker::parseFinished, delivered on the GUI thread.
    void parseFinished(modbus::parser::ParseResult result, quint64 requestId);

private:
    // Declaration order is load-bearing (reverse destruction): the worker's
    // deleteLater must be queued while the thread is still alive.
    std::shared_ptr<QThread> parseThread_;
    std::shared_ptr<modbus::parser::FrameParseWorker> parseWorker_;
};

} // namespace ui::application::analyzer
