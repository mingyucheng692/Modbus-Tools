/**
 * @file PollingController.cpp
 * @brief Implementation of PollingController.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#include "PollingController.h"
#include "RequestSubmissionService.h"
#include "AppConstants.h"
#include "../../logging/LogBridge.h"
#include <QCoreApplication>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace ui::application::modbus {

PollingController::PollingController(RequestSubmissionService* requestService,
                                     QObject* parent)
    : QObject(parent)
    , requestService_(requestService) {

    machine_ = new QStateMachine(this);

    idleState_ = new QState(machine_);
    pollingState_ = new QState(machine_);
    degradedState_ = new QState(machine_);
    escalatedState_ = new QState(machine_);

    machine_->setInitialState(idleState_);

    idleState_->addTransition(this, &PollingController::submitPollRequest, pollingState_);

    pollingState_->addTransition(this, &PollingController::escalated, escalatedState_);
    pollingState_->addTransition(this, &PollingController::degraded, degradedState_);

    degradedState_->addTransition(this, &PollingController::escalated, escalatedState_);
    degradedState_->addTransition(this, &PollingController::recovered, pollingState_);

    escalatedState_->addTransition(this, &PollingController::recovered, pollingState_);

    idleState_->addTransition(this, &PollingController::stopRequested, idleState_);
    pollingState_->addTransition(this, &PollingController::stopRequested, idleState_);
    degradedState_->addTransition(this, &PollingController::stopRequested, idleState_);
    escalatedState_->addTransition(this, &PollingController::stopRequested, idleState_);

    connect(pollingState_, &QState::entered, this, [this]() {
        spdlog::debug("PollingController: entered Polling state");
    });
    connect(degradedState_, &QState::entered, this, [this]() {
        spdlog::debug("PollingController: entered Degraded state");
    });
    connect(escalatedState_, &QState::entered, this, [this]() {
        spdlog::debug("PollingController: entered Escalated state");
    });
    connect(idleState_, &QState::entered, this, [this]() {
        spdlog::debug("PollingController: entered Idle state");
    });

    machine_->start();
}

void PollingController::setSessionConnected(bool connected) {
    setSessionConnectedInternal(connected, false);
}

void PollingController::setPollingInterval(int ms) {
    pollingIntervalMs_ = std::max(1, ms);
}

const PollContext& PollingController::context() const {
    return context_;
}

void PollingController::handleSessionConnected() {
    setSessionConnectedInternal(true, false);
}

void PollingController::handleSessionDisconnected(const QString& reason) {
    Q_UNUSED(reason);
    setSessionConnectedInternal(false, true);
}

void PollingController::handlePollRequest(const PollSpec& spec) {
    if (context_.requestInFlight) return;
    if (!context_.sessionConnected) return;

    context_.currentSpec = spec;

    buildAndSubmit();
}

void PollingController::buildAndSubmit() {
    if (!requestService_) return;

    auto result = requestService_->buildReadRequest(context_.currentSpec, RequestKind::Poll);
    if (!result.ok) {
        ui::common::TrafficEvent event;
        event.level = ui::common::TrafficEventLevel::Error;
        event.requestType = ui::common::TrafficRequestType::Poll;
        event.isPoll = true;
        event.summary = tr("Error: %1").arg(result.error);
        emit trafficEvent(event);
        return;
    }

    if (context_.summaryWindowStart == std::chrono::steady_clock::time_point{}) {
        context_.summaryWindowStart = std::chrono::steady_clock::now();
    }
    context_.requestInFlight = true;
    context_.suppressTrafficLog = true;
    transitionTo(PollState::Polling);
    emit submitPollRequest(result.pdu, context_.currentSpec.slaveId, result.requestId);
}

void PollingController::handleResponse(bool success, int rttMs, int retryCount,
                                       const QString& error) {
    context_.requestInFlight = false;
    context_.suppressTrafficLog = false;
    handlePollCompletion(success, rttMs, retryCount, error);
}

void PollingController::handlePollCompletion(bool success, int rttMs, int retryCount,
                                             const QString& error) {
    if (context_.summaryWindowStart == std::chrono::steady_clock::time_point{}) {
        context_.summaryWindowStart = std::chrono::steady_clock::now();
    }
    const auto now = std::chrono::steady_clock::now();
    context_.summaryRetryCount += std::max(0, retryCount);

    if (success) {
        ++context_.summarySuccessCount;
        context_.summaryTotalRttMs += (rttMs > 0 ? rttMs : 0);
        context_.lastSuccessTime = now;
        if (context_.consecutiveErrorCount > 0) {
            ui::common::TrafficEvent event;
            event.level = ui::common::TrafficEventLevel::Info;
            event.requestType = ui::common::TrafficRequestType::Poll;
            event.isPoll = true;
            event.summary = tr("Poll recovered after %1 consecutive failures")
                .arg(context_.consecutiveErrorCount);
            emit trafficEvent(event);
        }
        resetPollErrorTracking();
        if (context_.state == PollState::Degraded || context_.state == PollState::Escalated) {
            emit recovered();
            transitionTo(PollState::Polling);
        }
    } else {
        ++context_.summaryErrorCount;
        ++context_.consecutiveErrorCount;
        if (context_.failureStreakStartTime == std::chrono::steady_clock::time_point{}) {
            context_.failureStreakStartTime = now;
        }

        const QString normalizedError = error.toLower();
        int threshold = pollErrorThreshold();
        if (normalizedError.contains(QStringLiteral("timeout"))
            || normalizedError.contains(QStringLiteral("timed out"))) {
            threshold = std::max(2, threshold - 1);
        } else if (normalizedError.contains(QStringLiteral("busy"))
            || normalizedError.contains(QStringLiteral("tempor"))) {
            threshold = std::min(10, threshold + 1);
        }

        const bool zeroSuccessWindowExceeded =
            context_.failureStreakStartTime != std::chrono::steady_clock::time_point{}
            && (now - context_.failureStreakStartTime) >= std::chrono::milliseconds(3000);
        const bool connectionFault = !context_.sessionConnected;
        const bool escalateToError = connectionFault
            || zeroSuccessWindowExceeded
            || context_.consecutiveErrorCount >= threshold;
        const bool shouldLogEscalatedError = context_.state != PollState::Escalated
            || error != context_.lastErrorText
            || (now - context_.lastErrorLogTime) >= std::chrono::seconds(5);

        ui::common::TrafficEvent event;
        event.level = escalateToError
            ? ui::common::TrafficEventLevel::Error
            : ui::common::TrafficEventLevel::Warning;
        event.requestType = ui::common::TrafficRequestType::Poll;
        event.isPoll = true;

        if (escalateToError) {
            if (connectionFault) {
                event.summary = tr("Poll Error: Connection unavailable during polling (%1)")
                    .arg(error);
            } else if (context_.state != PollState::Escalated) {
                event.summary = tr("Poll Error escalated after %1 consecutive failures: %2")
                    .arg(context_.consecutiveErrorCount)
                    .arg(error);
            } else {
                event.summary = tr("Poll Error persists (%1 consecutive failures): %2")
                    .arg(context_.consecutiveErrorCount)
                    .arg(error);
            }

            if (shouldLogEscalatedError) {
                ui::logging::relay(event);
                emit trafficEvent(event);
                context_.lastErrorLogTime = now;
            }

            context_.lastErrorText = error;

            if (context_.state != PollState::Escalated) {
                emit escalated(
                    tr("Poll escalated after %1 consecutive failures").arg(context_.consecutiveErrorCount));
                transitionTo(PollState::Escalated);
            }
        } else {
            event.summary = tr("Poll Warning: %1 consecutive failure(s): %2")
                .arg(context_.consecutiveErrorCount)
                .arg(error);
            emit trafficEvent(event);
            context_.lastErrorText = error;
            if (context_.state == PollState::Polling) {
                emit degraded();
                transitionTo(PollState::Degraded);
            }
        }
    }
    flushPollSummary(false);
}

int PollingController::pollErrorThreshold() const {
    constexpr int kTargetUpgradeWindowMs = 3000;
    constexpr int kMinThreshold = 3;
    constexpr int kMaxThreshold = 10;

    const int intervalMs = std::max(pollingIntervalMs_, 1);
    const int roundedThreshold = (kTargetUpgradeWindowMs + intervalMs / 2) / intervalMs;
    return std::clamp(roundedThreshold, kMinThreshold, kMaxThreshold);
}

void PollingController::resetPollErrorTracking() {
    context_.consecutiveErrorCount = 0;
    context_.lastErrorText.clear();
    context_.lastErrorLogTime = std::chrono::steady_clock::time_point{};
    context_.failureStreakStartTime = std::chrono::steady_clock::time_point{};
}

void PollingController::flushPollSummary(bool force) {
    const int total = context_.summarySuccessCount + context_.summaryErrorCount;
    if (total <= 0) {
        return;
    }
    if (!force && context_.state == PollState::Escalated && context_.summarySuccessCount == 0) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    const bool due = (now - context_.summaryWindowStart) >= std::chrono::milliseconds(1000);
    if (!force && !due) {
        return;
    }

    PollSummary summary;
    summary.functionCode = context_.currentSpec.functionCode;
    summary.address = context_.currentSpec.startAddress;
    summary.quantity = context_.currentSpec.quantity;
    summary.slaveId = context_.currentSpec.slaveId;
    summary.successCount = context_.summarySuccessCount;
    summary.errorCount = context_.summaryErrorCount;
    summary.retryCount = context_.summaryRetryCount;
    summary.avgRttMs = context_.summarySuccessCount > 0
        ? static_cast<int>(context_.summaryTotalRttMs / context_.summarySuccessCount)
        : 0;

    emit summaryReady(summary);

    context_.summarySuccessCount = 0;
    context_.summaryErrorCount = 0;
    context_.summaryRetryCount = 0;
    context_.summaryTotalRttMs = 0;
    context_.summaryWindowStart = now;
}

void PollingController::stopPoll() {
    flushPollSummary(true);
    context_.requestInFlight = false;
    context_.suppressTrafficLog = false;
    resetPollErrorTracking();
    context_.lastSuccessTime = std::chrono::steady_clock::time_point{};
    transitionTo(PollState::Idle);
    emit stopRequested();
}

void PollingController::reset() {
    stopPoll();
    if (requestService_) {
        requestService_->clearAll();
    }
}

PollState PollingController::currentState() const {
    return context_.state;
}

bool PollingController::isSuppressingTrafficLog() const {
    return context_.suppressTrafficLog;
}

void PollingController::setSessionConnectedInternal(bool connected, bool stopActivePolling) {
    context_.sessionConnected = connected;
    if (!connected
        && stopActivePolling
        && (context_.requestInFlight || context_.state != PollState::Idle)) {
        stopPoll();
    }
}

void PollingController::transitionTo(PollState newState) {
    if (context_.state == newState) {
        return;
    }

    const auto oldState = context_.state;
    context_.state = newState;
    emit stateChanged(oldState, newState);
}

} // namespace ui::application::modbus
