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
#include <QScopedPointer>
#include <modbus/parser/ModbusFrameParser.h>
#include <modbus/base/ModbusTypes.h>

namespace modbus::core::parser {

/**
 * @brief Worker object designed to run in a background thread for frame parsing.
 * 
 * This class follows the Worker-Object pattern and PIMPL pattern.
 * It is detached from GUI dependencies and handles heavy parsing tasks.
 */
class FrameParseWorker : public QObject {
    Q_OBJECT

public:
    explicit FrameParseWorker(QObject* parent = nullptr);
    ~FrameParseWorker() override;

public slots:
    /**
     * @brief Enqueue a parse request.
     * @param input Raw hex string input from user.
     * @param type Protocol type (Tcp, Rtu, or Unknown for auto).
     * @param startAddress Starting address for response parsing.
     * @param order Register byte/word order.
     * @param requestId Unique ID to track the request.
     */
    void enqueueParse(const QString& input,
                      modbus::core::parser::ProtocolType type,
                      uint16_t startAddress,
                      modbus::base::RegisterOrder order,
                      quint64 requestId);

signals:
    /**
     * @brief Signal emitted when parsing is finished.
     */
    void parseFinished(const modbus::core::parser::ParseResult& result, quint64 requestId);

private:
    class Private;
    QScopedPointer<Private> d_ptr;
    Q_DISABLE_COPY(FrameParseWorker)
};

} // namespace modbus::core::parser
