// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include <QObject>
#include <QString>
#include <modbus/parser/ModbusFrameParser.h>
#include <modbus/base/ModbusTypes.h>

namespace modbus::parser {

// Role: Worker object running on a dedicated thread for parsing and decoding Modbus frame strings.
// Thread-safety: Thread-safe via Qt QueuedConnection; slots execute exclusively on worker's QThread.
// Ownership: Owned by FrameAnalyzerPresenter via std::shared_ptr and QObject hierarchy.
class FrameParseWorker : public QObject {
    Q_OBJECT

public:
    explicit FrameParseWorker(QObject* parent = nullptr);
    ~FrameParseWorker() noexcept override;

public slots:
    // Enqueues parse request onto worker thread; input must be pre-normalized hex string from UI layer.
    void enqueueParse(const QString& input,
                      ProtocolType type,
                      uint16_t startAddress,
                      modbus::base::RegisterOrder order,
                      quint64 requestId);

signals:
    void parseFinished(const ParseResult& result, quint64 requestId);

private:
    // Pending request state — only touched on the worker thread.
    QString pendingInput_;
    ProtocolType pendingType_ = ProtocolType::Unknown;
    uint16_t pendingStartAddress_ = 0;
    modbus::base::RegisterOrder pendingOrder_ = modbus::base::RegisterOrder::ABCD;
    quint64 pendingRequestId_ = 0;
    bool hasPendingRequest_ = false;
    bool processing_ = false;

    void processPending();

    Q_DISABLE_COPY(FrameParseWorker)
};

} // namespace modbus::parser
