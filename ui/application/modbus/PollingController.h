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
#include <QStateMachine>
#include <QState>
#include <chrono>
#include <cstdint>
#include "modbus/base/ModbusFrame.h"
#include "../../common/TrafficEvent.h"
#include "ModbusTypes.h"

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

signals:
    void submitPollRequest(const ::modbus::base::Pdu& pdu, int slaveId, int requestId);
    void trafficEvent(const ui::common::TrafficEvent& event);
    void stateChanged(PollState oldState, PollState newState);
    void summaryReady(const PollSummary& summary);
    void escalated(const QString& message);
    void degraded();
    void recovered();
    void stopRequested();

private:
    void buildAndSubmit();
    void handlePollCompletion(bool success, int rttMs, int retryCount, const QString& error);
    int pollErrorThreshold() const;
    void resetPollErrorTracking();
    void flushPollSummary(bool force);

    RequestSubmissionService* requestService_ = nullptr;

    QStateMachine* machine_ = nullptr;
    QState* idleState_ = nullptr;
    QState* pollingState_ = nullptr;
    QState* degradedState_ = nullptr;
    QState* escalatedState_ = nullptr;
    PollState currentState_ = PollState::Idle;

    bool sessionConnected_ = false;
    int pollingIntervalMs_ = 1000;

    bool pollInFlight_ = false;
    bool suppressPollTrafficLog_ = false;
    std::chrono::steady_clock::time_point pollSummaryWindowStart_{};
    int pollSummarySuccessCount_ = 0;
    int pollSummaryErrorCount_ = 0;
    int pollSummaryRetryCount_ = 0;
    qint64 pollSummaryTotalRttMs_ = 0;
    PollSpec currentPollSpec_;
    int pollConsecutiveErrorCount_ = 0;
    bool pollErrorEscalated_ = false;
    QString pollLastErrorText_;
    std::chrono::steady_clock::time_point pollLastErrorLogTime_{};
    std::chrono::steady_clock::time_point pollLastSuccessTime_{};
    std::chrono::steady_clock::time_point pollFailureStreakStartTime_{};
};

} // namespace ui::application::modbus
