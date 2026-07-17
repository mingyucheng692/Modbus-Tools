/**
 * @file PollingController.h
 * @brief Polling scheduler with error escalation state machine.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <QObject>
#include <chrono>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "../../common/TrafficEvent.h"
#include "ModbusTypes.h"
#include "polling/PollingStrategy.h"

namespace ui::application::modbus {

class RequestSubmissionService;

enum class PollState {
    Idle,
    Polling,
    Degraded,
    Escalated
};

struct PollSummary {
    uint8_t functionCode = 0;
    int address = 0;
    int quantity = 0;
    int slaveId = 0;
    int successCount = 0;
    int errorCount = 0;
    int retryCount = 0;
    int avgRttMs = 0;
};

struct PollContext {
    PollState state = PollState::Idle;
    bool sessionConnected = false;
    bool requestInFlight = false;
    bool suppressTrafficLog = false;
    PollSpec currentSpec{};
    int consecutiveErrorCount = 0;
    QString lastErrorText;
    std::chrono::steady_clock::time_point lastErrorLogTime{};
    std::chrono::steady_clock::time_point lastSuccessTime{};
    std::chrono::steady_clock::time_point failureStreakStartTime{};
    std::chrono::steady_clock::time_point summaryWindowStart{};
    int summarySuccessCount = 0;
    int summaryErrorCount = 0;
    int summaryRetryCount = 0;
    qint64 summaryTotalRttMs = 0;
};

class PollingController : public QObject {
    Q_OBJECT

public:
    explicit PollingController(RequestSubmissionService* requestService,
                               QObject* parent = nullptr);

    void setSessionConnected(bool connected);
    void setPollingInterval(int ms);

    void handlePollRequest(const PollSpec& spec);
    void handleResponse(bool success, int rttMs, int retryCount, const QString& error);
    void stopPoll();
    void reset();

    PollState currentState() const;
    bool isSuppressingTrafficLog() const;
    const PollContext& context() const;

signals:
    void submitPollRequest(const ::modbus::base::Pdu& pdu, int slaveId, int requestId);
    void trafficEvent(const ui::common::TrafficEvent& event);
    void stateChanged(PollState oldState, PollState newState);
    void summaryReady(const PollSummary& summary);
    void stopRequested();

public slots:
    void handleSessionConnected();
    void handleSessionDisconnected(const QString& reason = QString());

private:
    void buildAndSubmit();
    void handlePollCompletion(bool success, int rttMs, int retryCount, const QString& error);
    int pollErrorThreshold() const;
    void resetPollErrorTracking();
    void flushPollSummary(bool force);
    void setSessionConnectedInternal(bool connected, bool stopActivePolling);
    void transitionTo(PollState newState);

    RequestSubmissionService* requestService_ = nullptr;

    int pollingIntervalMs_ = 1000;
    PollContext context_{};
};

} // namespace ui::application::modbus
