/**
 * @file FrameParseWorker.cpp
 * @brief Implementation of FrameParseWorker.
 * 
 * Copyright (c) 2025 - present mingyucheng692
 * 
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "FrameParseWorker.h"
#include <QRegularExpression>
#include <QCoreApplication>
#include <QDateTime>
#include <QMetaObject>

namespace modbus::core::parser {

class FrameParseWorker::Private {
public:
    explicit Private(FrameParseWorker* q) : q_ptr(q) {}

    // --- State ---
    QString pendingInput;
    ProtocolType pendingType = ProtocolType::Unknown;
    uint16_t pendingStartAddress = 0;
    modbus::base::RegisterOrder pendingOrder = modbus::base::RegisterOrder::ABCD;
    quint64 pendingRequestId = 0;
    bool hasPendingRequest = false;
    bool processing = false;

    // --- Helpers ---
    [[nodiscard]] QString normalizeHexInput(const QString& input) const;
    [[nodiscard]] uint32_t maskForBits(int bitWidth) const;
    [[nodiscard]] ParseResult buildResult(const QString& input,
                                          ProtocolType type,
                                          uint16_t startAddress,
                                          modbus::base::RegisterOrder order) const;

    void processPending();

private:
    FrameParseWorker* q_ptr;
};

QString FrameParseWorker::Private::normalizeHexInput(const QString& input) const {
    QString text = input;
    // Remove bracketed metadata segments such as "[12:39:35.668]" / "[RX]".
    text.remove(QRegularExpression(QStringLiteral("\\[[^\\]]*\\]")));

    const QStringList rawTokens = text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    QString normalized;
    normalized.reserve(rawTokens.size() * 2);

    for (const QString& rawToken : rawTokens) {
        QString token = rawToken.trimmed();
        if (token.isEmpty()) continue;

        const QString upper = token.toUpper();
        if (upper == "RX" || upper == "TX" || upper == "FAIL" || upper == "RTT") continue;

        // Skip obvious timestamp/date-like tokens.
        if (token.contains(':') || token.contains('.') || token.contains('-')) continue;

        if (token.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) {
            token = token.mid(2);
        }

        token.remove(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]")));
        if (token.size() < 2) continue;
        if (token.size() % 2 != 0) token.chop(1);
        if (!token.isEmpty()) normalized.append(token);
    }

    // Fallback for plain contiguous hex input.
    if (normalized.isEmpty()) {
        QString plain = input;
        plain.remove(QRegularExpression(QStringLiteral("[^0-9A-Fa-f]")));
        if (plain.size() % 2 != 0) plain.chop(1);
        normalized = plain;
    }
    return normalized;
}

uint32_t FrameParseWorker::Private::maskForBits(int bitWidth) const {
    if (bitWidth >= 32) return 0xFFFFFFFFu;
    return (1u << bitWidth) - 1u;
}

ParseResult FrameParseWorker::Private::buildResult(const QString& input,
                                                   ProtocolType type,
                                                   uint16_t startAddress,
                                                   modbus::base::RegisterOrder order) const {
    const QString hexStr = normalizeHexInput(input);

    ParseResult result;
    result.timestamp = QDateTime::currentDateTime();
    if (hexStr.isEmpty()) {
        result.isValid = false;
        result.error = QCoreApplication::translate("FrameParseWorker", "Error: Empty input");
        return result;
    }

    const QByteArray frame = QByteArray::fromHex(hexStr.toLatin1());
    const bool force = (type != ProtocolType::Unknown);
    return ModbusFrameParser::parse(frame, type, startAddress, 0, force, order);
}

void FrameParseWorker::Private::processPending() {
    while (hasPendingRequest) {
        const QString input = pendingInput;
        const ProtocolType type = pendingType;
        const uint16_t startAddress = pendingStartAddress;
        const quint64 requestId = pendingRequestId;
        const modbus::base::RegisterOrder order = pendingOrder;
        hasPendingRequest = false;

        emit q_ptr->parseFinished(buildResult(input, type, startAddress, order), requestId);
    }
    processing = false;
}

// --- FrameParseWorker Public Implementation ---

FrameParseWorker::FrameParseWorker(QObject* parent)
    : QObject(parent), d_ptr(new Private(this)) {}

FrameParseWorker::~FrameParseWorker() = default;

void FrameParseWorker::enqueueParse(const QString& input,
                                    modbus::core::parser::ProtocolType type,
                                    uint16_t startAddress,
                                    modbus::base::RegisterOrder order,
                                    quint64 requestId) {
    d_ptr->pendingInput = input;
    d_ptr->pendingType = type;
    d_ptr->pendingStartAddress = startAddress;
    d_ptr->pendingOrder = order;
    d_ptr->pendingRequestId = requestId;
    d_ptr->hasPendingRequest = true;

    if (d_ptr->processing) return;

    d_ptr->processing = true;
    QMetaObject::invokeMethod(this, [this]() { d_ptr->processPending(); }, Qt::QueuedConnection);
}

} // namespace modbus::core::parser
