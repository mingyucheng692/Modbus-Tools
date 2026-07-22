/**
 * @file FrameParseWorker.cpp
 * @brief Implementation of FrameParseWorker.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FrameParseWorker.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QMetaObject>

namespace modbus::parser {

FrameParseWorker::FrameParseWorker(QObject* parent)
    : QObject(parent) {}

FrameParseWorker::~FrameParseWorker() = default;

void FrameParseWorker::enqueueParse(const QString& input,
                                    ProtocolType type,
                                    uint16_t startAddress,
                                    modbus::base::RegisterOrder order,
                                    quint64 requestId) {
    pendingInput_ = input;
    pendingType_ = type;
    pendingStartAddress_ = startAddress;
    pendingOrder_ = order;
    pendingRequestId_ = requestId;
    hasPendingRequest_ = true;

    if (processing_) return;

    processing_ = true;
    QMetaObject::invokeMethod(this, [this]() { processPending(); }, Qt::QueuedConnection);
}

void FrameParseWorker::processPending() {
    while (hasPendingRequest_) {
        const QString input = pendingInput_;
        const ProtocolType type = pendingType_;
        const uint16_t startAddress = pendingStartAddress_;
        const quint64 requestId = pendingRequestId_;
        const modbus::base::RegisterOrder order = pendingOrder_;
        hasPendingRequest_ = false;

        // Input is assumed already normalized by the UI layer.
        ParseResult result;
        result.timestamp = QDateTime::currentDateTimeUtc();
        if (input.isEmpty()) {
            result.isValid = false;
            result.error = QCoreApplication::translate("FrameParseWorker", "Error: Empty input");
        } else {
            const QByteArray frame = QByteArray::fromHex(input.toLatin1());
            const bool force = (type != ProtocolType::Unknown);
            result = parse(frame, type, startAddress, 0, force, order);
        }

        emit parseFinished(result, requestId);
    }
    processing_ = false;
}

} // namespace modbus::parser
