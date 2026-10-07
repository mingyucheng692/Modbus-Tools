// Copyright (c) 2025 - present mingyucheng692. Licensed under the MIT License.

#pragma once

#include "infra/logging/TraceContext.h"
#include <atomic>
#include <spdlog/spdlog.h>
#include <string_view>

namespace modbus::session {

// Role: CRTP base providing thread-safe atomic state transitions and validation logging.
// Thread-safety: Thread-safe via atomic compare-exchange operations.
// Ownership: Base class embedded into derived state machines (ConnectionStateMachine, RequestStateMachine).
template<typename Derived, typename StateEnum>
class StateMachineBase {
public:
    using State = StateEnum;

    bool isValidTransition(State from, State to) const {
        for (const auto& t : Derived::kValidTransitions) {
            if (t.from == from && t.to == to) {
                return true;
            }
        }
        return false;
    }

    bool tryTransition(State newState, const char* reason) {
        State oldState = state_.load(std::memory_order_relaxed);

        while (true) {
            if (oldState == newState) {
                return true;
            }

            if (!isValidTransition(oldState, newState)) {
                // trace_id is 0 for connection-lifecycle transitions (no
                // request in flight); non-zero when a request drives the
                // transition. Never required here — 0 is a valid value.
                SPDLOG_ERROR("{}: invalid transition {} -> {} ({}) trace_id={}",
                              Derived::kName,
                              Derived::toString(oldState),
                              Derived::toString(newState),
                              reason ? reason : "",
                              static_cast<unsigned long long>(modbus::trace::currentTraceId));
                return false;
            }

            if (state_.compare_exchange_weak(oldState, newState,
                    std::memory_order_acq_rel, std::memory_order_relaxed)) {
                SPDLOG_DEBUG("{}: {} -> {} ({}) trace_id={}",
                              Derived::kName,
                              Derived::toString(oldState),
                              Derived::toString(newState),
                              reason ? reason : "",
                              static_cast<unsigned long long>(modbus::trace::currentTraceId));
                return true;
            }
        }
    }

    [[nodiscard]] State currentState() const {
        return state_.load(std::memory_order_acquire);
    }

    void forceReset(State newState) {
        state_.store(newState, std::memory_order_release);
    }

protected:
    std::atomic<State> state_{Derived::kInitialState};
};

} // namespace modbus::session
