/**
 * @file StateMachineBase.h
 * @brief CRTP base class providing shared state-machine logic (CAS transitions, reset, queries).
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

#include <atomic>
#include <spdlog/spdlog.h>
#include <string_view>

namespace modbus::session {

/**
 * @brief CRTP base that supplies the shared state-machine implementation.
 *
 * Derived classes must expose:
 *   - `enum class State`              — the set of states
 *   - `struct Transition { State from; State to; };`
 *   - `static constexpr std::array kValidTransitions` — allowed transitions
 *   - `static constexpr const char* kName`             — log tag
 *   - `static constexpr State kInitialState`           — power-on state
 *   - `static constexpr const char* toString(State)`   — state label for logs
 */
template<typename Derived>
class StateMachineBase {
public:
    using State = typename Derived::State;

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
                spdlog::error("{}: invalid transition {} -> {} ({})",
                              Derived::kName,
                              Derived::toString(oldState),
                              Derived::toString(newState),
                              reason ? reason : "");
                return false;
            }

            if (state_.compare_exchange_weak(oldState, newState,
                    std::memory_order_acq_rel, std::memory_order_relaxed)) {
                spdlog::debug("{}: {} -> {} ({})",
                              Derived::kName,
                              Derived::toString(oldState),
                              Derived::toString(newState),
                              reason ? reason : "");
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
