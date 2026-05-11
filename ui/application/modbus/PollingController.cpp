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
        currentState_ = PollState::Polling;
        spdlog::debug("PollingController: entered Polling state");
    });
    connect(degradedState_, &QState::entered, this, [this]() {
        auto old = currentState_;
        currentState_ = PollState::Degraded;
        emit stateChanged(old, PollState::Degraded);
        spdlog::debug("PollingController: entered Degraded state");
    });
    connect(escalatedState_, &QState::entered, this, [this]() {
        auto old = currentState_;
        currentState_ = PollState::Escalated;
        emit stateChanged(old, PollState::Escalated);
        spdlog::debug("PollingController: entered Escalated state");
    });
    connect(idleState_, &QState::entered, this, [this]() {
        auto old = currentState_;
        currentState_ = PollState::Idle;
        emit stateChanged(old, PollState::Idle);
        spdlog::debug("PollingController: entered Idle state");
    });

    machine_->start();
}

void PollingController::setSessionConnected(bool connected) {
    sessionConnected_ = connected;
}

void PollingController::setPollingInterval(int ms) {
    pollingIntervalMs_ = std::max(1, ms);
}

void PollingController::handlePollRequest(const PollSpec& spec) {
    if (pollInFlight_) return;
    if (!sessionConnected_) return;

    currentPollSpec_ = spec;

    buildAndSubmit();
}

void PollingController::buildAndSubmit() {
    if (!requestService_) return;

    auto result = requestService_->buildReadRequest(currentPollSpec_, RequestKind::Poll);
    if (!result.ok) {
        ui::common::TrafficEvent event;
        event.level = ui::common::TrafficEventLevel::Error;
        event.requestType = ui::common::TrafficRequestType::Poll;
        event.isPoll = true;
        event.summary = tr("Error: %1").arg(result.error);
        emit trafficEvent(event);
        return;
    }

    if (pollSummaryWindowStart_ == std::chrono::steady_clock::time_point{}) {
        pollSummaryWindowStart_ = std::chrono::steady_clock::now();
    }
    pollInFlight_ = true;
    suppressPollTrafficLog_ = true;
    emit submitPollRequest(result.pdu, currentPollSpec_.slaveId, result.requestId);
}

void PollingController::handleResponse(bool success, int rttMs, int retryCount,
                                       const QString& error) {
    pollInFlight_ = false;
    suppressPollTrafficLog_ = false;
    handlePollCompletion(success, rttMs, retryCount, error);
}

void PollingController::handlePollCompletion(bool success, int rttMs, int retryCount,
                                             const QString& error) {
    if (pollSummaryWindowStart_ == std::chrono::steady_clock::time_point{}) {
        pollSummaryWindowStart_ = std::chrono::steady_clock::now();
    }
    const auto now = std::chrono::steady_clock::now();
    pollSummaryRetryCount_ += std::max(0, retryCount);

    if (success) {
        ++pollSummarySuccessCount_;
        pollSummaryTotalRttMs_ += (rttMs > 0 ? rttMs : 0);
        pollLastSuccessTime_ = now;
        if (pollConsecutiveErrorCount_ > 0) {
            ui::common::TrafficEvent event;
            event.level = ui::common::TrafficEventLevel::Info;
            event.requestType = ui::common::TrafficRequestType::Poll;
            event.isPoll = true;
            event.summary = tr("Poll recovered after %1 consecutive failures")
                .arg(pollConsecutiveErrorCount_);
            emit trafficEvent(event);
        }
        resetPollErrorTracking();
        if (currentState_ == PollState::Degraded || currentState_ == PollState::Escalated) {
            emit recovered();
        }
    } else {
        ++pollSummaryErrorCount_;
        ++pollConsecutiveErrorCount_;
        if (pollFailureStreakStartTime_ == std::chrono::steady_clock::time_point{}) {
            pollFailureStreakStartTime_ = now;
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
            pollFailureStreakStartTime_ != std::chrono::steady_clock::time_point{}
            && (now - pollFailureStreakStartTime_) >= std::chrono::milliseconds(3000);
        const bool connectionFault = !sessionConnected_;
        const bool escalateToError = connectionFault
            || zeroSuccessWindowExceeded
            || pollConsecutiveErrorCount_ >= threshold;
        const bool shouldLogEscalatedError = !pollErrorEscalated_
            || error != pollLastErrorText_
            || (now - pollLastErrorLogTime_) >= std::chrono::seconds(5);

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
            } else if (!pollErrorEscalated_) {
                event.summary = tr("Poll Error escalated after %1 consecutive failures: %2")
                    .arg(pollConsecutiveErrorCount_)
                    .arg(error);
            } else {
                event.summary = tr("Poll Error persists (%1 consecutive failures): %2")
                    .arg(pollConsecutiveErrorCount_)
                    .arg(error);
            }

            if (shouldLogEscalatedError) {
                emit trafficEvent(event);
                pollLastErrorLogTime_ = now;
            }

            pollErrorEscalated_ = true;
            pollLastErrorText_ = error;

            if (currentState_ != PollState::Escalated) {
                emit escalated(
                    tr("Poll escalated after %1 consecutive failures").arg(pollConsecutiveErrorCount_));
            }
        } else {
            emit trafficEvent(event);
            pollLastErrorText_ = error;
            if (currentState_ == PollState::Polling) {
                emit degraded();
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
    pollConsecutiveErrorCount_ = 0;
    pollErrorEscalated_ = false;
    pollLastErrorText_.clear();
    pollLastErrorLogTime_ = std::chrono::steady_clock::time_point{};
    pollFailureStreakStartTime_ = std::chrono::steady_clock::time_point{};
}

void PollingController::flushPollSummary(bool force) {
    const int total = pollSummarySuccessCount_ + pollSummaryErrorCount_;
    if (total <= 0) {
        return;
    }
    const auto now = std::chrono::steady_clock::now();
    const bool due = (now - pollSummaryWindowStart_) >= std::chrono::milliseconds(1000);
    if (!force && !due) {
        return;
    }

    PollSummary summary;
    summary.functionCode = currentPollSpec_.functionCode;
    summary.address = currentPollSpec_.startAddress;
    summary.quantity = currentPollSpec_.quantity;
    summary.slaveId = currentPollSpec_.slaveId;
    summary.successCount = pollSummarySuccessCount_;
    summary.errorCount = pollSummaryErrorCount_;
    summary.retryCount = pollSummaryRetryCount_;
    summary.avgRttMs = pollSummarySuccessCount_ > 0
        ? static_cast<int>(pollSummaryTotalRttMs_ / pollSummarySuccessCount_)
        : 0;

    emit summaryReady(summary);

    pollSummarySuccessCount_ = 0;
    pollSummaryErrorCount_ = 0;
    pollSummaryRetryCount_ = 0;
    pollSummaryTotalRttMs_ = 0;
    pollSummaryWindowStart_ = now;
}

void PollingController::stopPoll() {
    flushPollSummary(true);
    pollInFlight_ = false;
    suppressPollTrafficLog_ = false;
    resetPollErrorTracking();
    pollLastSuccessTime_ = std::chrono::steady_clock::time_point{};
    emit stopRequested();
}

void PollingController::reset() {
    stopPoll();
    if (requestService_) {
        requestService_->clearAll();
    }
}

PollState PollingController::currentState() const {
    return currentState_;
}

bool PollingController::isSuppressingTrafficLog() const {
    return suppressPollTrafficLog_;
}

} // namespace ui::application::modbus
