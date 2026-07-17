# ADR 0003: State Machine Pattern for Modbus Session and Polling

**Date:** 2026-06-28

**Status:** Accepted (revised 2026-07-17 to reflect actual implementation)

---

## Context

The Modbus-Tools application manages two distinct stateful workflows:
1. **Session connection lifecycle**: Disconnected → Connecting → Connected → Reconnecting → Disconnecting → Failed, with error/reconnect paths.
2. **Request lifecycle**: Idle → Sending → Waiting → Completed/Failed/Aborted.

Without explicit state machines, these workflows were implemented with boolean flags and ad-hoc if-else chains, making them fragile and hard to extend.

## Decision

We use **constexpr transition tables + lock-free CAS (compare-and-swap)** for core-layer state machines, and **Qt's `QStateMachine`** for UI-layer polling escalation.

### Core-layer state machines (constexpr + CAS)

- `ConnectionStateMachine` in `core/modbus/session/` manages the session connection lifecycle.
- `RequestStateMachine` in `core/modbus/session/` manages the per-request lifecycle.
- Both inherit from `StateMachineBase<Derived>` (CRTP template) which provides the shared `tryTransition` (CAS loop with `memory_order_acq_rel`), `currentState` (`memory_order_acquire`), `forceReset` (`memory_order_release`), and `isValidTransition` (linear scan over constexpr transition table).

Each derived class exposes:
- `enum class State` — the set of states
- `static constexpr std::array kValidTransitions` — allowed transitions (compile-time validated via `static_assert`)
- `static constexpr const char* toString(State)` — state label for logging
- `static constexpr State kInitialState` — power-on state

### UI-layer state machine (QStateMachine)

- `PollingController` in `ui/application/modbus/` uses Qt's `QStateMachine` for the polling escalation state machine (idle → polling → degraded → escalated → recovered).

### Why two approaches

- **Core layer** requires thread-safe, lock-free state transitions (called from worker thread + UI thread). `QStateMachine` is event-loop-bound (asynchronous, single-threaded) and unsuitable. The constexpr + CAS approach gives deterministic, thread-safe transitions with zero heap allocation.
- **UI layer** polling escalation is single-threaded and benefits from `QStateMachine`'s signal-driven transitions and Qt Creator visualization.

## Alternatives Considered

1. **Boolean flags + manual if-else** — simple but scales poorly; adding a new state requires touching multiple conditionals.
2. **Boost.Statechart** — powerful but adds a heavy dependency.
3. **QStateMachine for all (original plan)** — rejected for core layer: event-loop-bound, asynchronous, single-threaded only.
4. **Constexpr transition table + CAS (chosen for core)** — thread-safe, lock-free, compile-time validated, zero allocation.

## Consequences

- **Positive**: Core state transitions are thread-safe without mutexes (CAS).
- **Positive**: Invalid transitions are caught at compile time (`static_assert`) and runtime (spdlog::error).
- **Positive**: CRTP template eliminates ~112 LOC of duplicated CAS/transition code.
- **Positive**: UI polling state machine remains visualizable in Qt Creator.
- **Negative**: Two state machine paradigms in the codebase (core vs UI).
- **Negative**: CAS transition table requires manual maintenance of the `kValidTransitions` array.

## Note on SessionConnectionStateMachine

The UI-layer `SessionConnectionStateMachine` (a `QObject`-based parallel FSM that bridged core session state to UI signals) was evaluated for removal (P0-3). It was deferred as a high-risk refactor with no correctness benefit. The bridge remains until a dedicated UI-state-derivation refactor is undertaken.
