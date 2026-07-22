/**
 * @file FrameParseWorker.h
 * @brief Background worker for Modbus frame parsing.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <QString>
#include <modbus/parser/ModbusFrameParser.h>
#include <modbus/base/ModbusTypes.h>

namespace modbus::parser {

/**
 * @brief Worker object designed to run in a background thread for frame parsing.
 *
 * @par Worker-Object pattern
 *      Slots are queued via QMetaObject::invokeMethod / QueuedConnection so
 *      the worker can live on a dedicated QThread. Inputs MUST be
 *      pre-normalized by the UI layer (no brackets / timestamps / 0x prefixes
 *      / non-hex characters); FrameParseWorker does not perform input
 *      sanitization itself.
 *
 * @par Why no PIMPL
 *      The previous QScopedPointer<Private> indirection wrapped 6 POD members
 *      with no forward-declaration or ABI-stability requirement, so it only
 *      added boilerplate. Members are now declared directly.
 */
class FrameParseWorker : public QObject {
    Q_OBJECT

public:
    explicit FrameParseWorker(QObject* parent = nullptr);
    ~FrameParseWorker() noexcept override;

public slots:
    /**
     * @brief Enqueue a parse request.
     * @param input Pre-normalized hex string from the UI layer.
     * @param type Protocol type (Tcp, Rtu, or Unknown for auto).
     * @param startAddress Starting address for response parsing.
     * @param order Register byte/word order.
     * @param requestId Unique ID to track the request.
     */
    void enqueueParse(const QString& input,
                      ProtocolType type,
                      uint16_t startAddress,
                      modbus::base::RegisterOrder order,
                      quint64 requestId);

signals:
    /**
     * @brief Signal emitted when parsing is finished.
     */
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
