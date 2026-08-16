/**
 * @file FrameAnalyzerPresenter.cpp
 * @brief Implementation of FrameAnalyzerPresenter.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FrameAnalyzerPresenter.h"
#include "common/ThreadGuard.h"
#include "modbus/parser/FrameParseWorker.h"
#include <QMetaObject>

namespace ui::application::analyzer {

FrameAnalyzerPresenter::FrameAnalyzerPresenter(QObject* parent)
    : QObject(parent)
{
    // Cross-thread signal payloads must be registered exactly once for the
    // queued worker->GUI delivery; this is the single registration point
    // since the thread/worker pair moved out of FrameAnalyzerWidget
    // (missing registration = silently dropped signals).
    qRegisterMetaType<modbus::parser::ProtocolType>();
    qRegisterMetaType<modbus::parser::ParseResult>();
    qRegisterMetaType<modbus::base::RegisterOrder>();

    parseThread_ = std::shared_ptr<QThread>(
        new QThread(), &core::common::ThreadGuard::releaseThread);
    parseWorker_ = std::shared_ptr<modbus::parser::FrameParseWorker>(
        new modbus::parser::FrameParseWorker(),
        &core::common::ThreadGuard::releaseWorker<modbus::parser::FrameParseWorker>);
    parseWorker_->moveToThread(parseThread_.get());

    connect(parseWorker_.get(), &modbus::parser::FrameParseWorker::parseFinished,
            this, &FrameAnalyzerPresenter::parseFinished);

    parseThread_->start();
}

FrameAnalyzerPresenter::~FrameAnalyzerPresenter()
{
    // Teardown runs via the shared_ptr deleters -> core::common::ThreadGuard.
    // Destruction order (worker before thread, see member declaration) queues
    // the worker's deleteLater onto the still-live worker thread, then
    // releaseThread() quits + joins it. The worker->this connection is
    // severed automatically by ~QObject, so an in-flight parse cannot
    // deliver into a dangling presenter.
}

void FrameAnalyzerPresenter::enqueueParse(const QString& input,
                                          modbus::parser::ProtocolType type,
                                          uint16_t startAddress,
                                          modbus::base::RegisterOrder order,
                                          quint64 requestId)
{
    if (!parseWorker_) {
        return;
    }
    // Marshal onto the worker thread: FrameParseWorker's pending-request
    // state is owned by that thread. (The widget previously called the slot
    // directly from the GUI thread, a latent data race.)
    QMetaObject::invokeMethod(parseWorker_.get(), [=]() {
        parseWorker_->enqueueParse(input, type, startAddress, order, requestId);
    }, Qt::QueuedConnection);
}

} // namespace ui::application::analyzer
