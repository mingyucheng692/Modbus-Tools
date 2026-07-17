/**
 * @file RequestCoordinator.h
 * @brief Request orchestration coordinator — extracts request-flow logic from BaseModbusPage.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <QByteArray>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "modbus/parser/ModbusFrameParser.h"
#include "ModbusSessionPresenter.h"

namespace ui::widgets {
class FunctionWidget;
}

namespace ui::application::modbus {

class RequestSubmissionService;
class PollingController;
class TrafficLogController;

/**
 * @brief Coordinates the request lifecycle between View signals and backend services.
 *
 * Responsibilities:
 *   - Read request orchestration: PollSpec construction → PDU build → log → submit
 *   - Write request orchestration: PollSpec construction → PDU build → log → submit
 *   - Raw send orchestration: validation → log → submit
 *   - Poll request orchestration: PollSpec construction → schedule
 *   - Request completion: protocol type determination → linkage data forwarding
 *
 * Lives on the GUI thread. Uses ConnectionAlert (which wraps QMessageBox)
 * for connection-required prompts, so it transitively depends on QWidget
 * via the alert helper. This is an acknowledged UI-layer coupling.
 */
class RequestCoordinator : public QObject {
    Q_OBJECT

public:
    explicit RequestCoordinator(ModbusSessionPresenter* presenter,
                                RequestSubmissionService* requestService,
                                PollingController* pollingController,
                                TrafficLogController* trafficLogController,
                                SessionMode sessionMode,
                                QObject* parent = nullptr);

    void handleReadRequest(uint8_t fc, int addr, int qty, int slaveId);
    void handleWriteRequest(uint8_t fc, int addr, const QString& dataStr,
                            const QString& fmt, int slaveId, int quantity);
    void handleRawSendRequest(const QByteArray& data);
    void handlePollRequest(uint8_t fc, int addr, int qty, int intervalMs);
    void handleRequestFinished(int requestId,
                               const ::modbus::session::ModbusResponse& response);

signals:
    void linkageDataReceived(const ::modbus::base::Pdu& pdu,
                             ::modbus::core::parser::ProtocolType protocol,
                             uint16_t addr);

private:
    [[nodiscard]] bool ensureConnected() const;

    ModbusSessionPresenter* presenter_;
    RequestSubmissionService* requestService_;
    PollingController* pollingController_;
    TrafficLogController* trafficLogController_;
    SessionMode sessionMode_;
};

} // namespace ui::application::modbus
