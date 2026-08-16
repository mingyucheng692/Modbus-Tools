/**
 * @file FrameAnalyzerPresenter.h
 * @brief Application-layer presenter owning the frame-parse thread/worker pair.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

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

/**
 * @brief Owns the background QThread + FrameParseWorker used by
 *        FrameAnalyzerWidget for logical frame parsing.
 *
 * The widget previously created and tore down this pair itself, duplicating
 * the project's thread-cleanup sequence. Ownership now lives here; teardown
 * goes exclusively through core::common::ThreadGuard, and the widget keeps
 * only rendering and interaction.
 *
 * @par Threading contract
 *      - Widget -> Presenter: same-thread calls/signals (DirectConnection).
 *      - Presenter -> Worker: QueuedConnection (worker lives on its thread);
 *        FrameParseWorker's pending state is only touched on that thread.
 *      - Worker -> Widget results are relayed via the parseFinished signal
 *        (queued cross-thread, then direct to the widget on the GUI thread).
 *
 * @par Lifetime
 *      Intended to be created as a QObject child of the widget
 *      (`new FrameAnalyzerPresenter(widget)`). The destructor performs the
 *      ThreadGuard teardown: worker first (deleteLater queued onto the still
 *      live worker thread), thread second (quit + bounded wait, with the
 *      finished->deleteLater fallback). Member order below is load-bearing —
 *      see ThreadGuard.h.
 */
class FrameAnalyzerPresenter : public QObject {
    Q_OBJECT
public:
    explicit FrameAnalyzerPresenter(QObject* parent = nullptr);
    ~FrameAnalyzerPresenter() override;

    /// Queues a parse request onto the worker thread.
    /// @param input Pre-normalized hex string (see FrameParseWorker contract).
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
