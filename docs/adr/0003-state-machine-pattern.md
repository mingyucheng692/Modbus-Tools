# ADR 0003: State Machine Pattern for Modbus Session and Polling

**Date:** 2026-06-28

**Status:** Accepted

---

## Context

The Modbus-Tools application manages two distinct stateful workflows:
1. **Session connection lifecycle**: idle → connecting → connected → disconnecting → idle, with error/reconnect paths.
2. **Polling escalation**: idle → polling → degraded → escalated → recovered → polling, driven by consecutive error counts.

Without explicit state machines, these workflows were implemented with boolean flags and ad-hoc if-else chains, making them fragile and hard to extend.

## Decision

We use Qt's `QStateMachine` (from `Qt6::StateMachine`) for all stateful workflows:
- `ConnectionStateMachine` in `core/modbus/session/` manages the session lifecycle.
- `PollingController` in `ui/application/modbus/` manages the polling escalation state machine.
- `SessionConnectionStateMachine` in `ui/application/modbus/` bridges the core session state machine to UI signals.

Each state machine is a self-contained class:
- States are `QState*` members.
- Transitions are signal-driven (`addTransition(this, &Signal, targetState)`).
- Side effects (logging, UI updates) are connected to `QState::entered` signals.

## Alternatives Considered

1. **Boolean flags + manual if-else** — simple but scales poorly; adding a new state requires touching multiple conditionals.
2. **Boost.Statechart** — powerful but adds a heavy dependency.
3. **Qt State Machine (chosen)** — already available via Qt6, well-integrated with signals/slots, lightweight.

## Consequences

- **Positive**: Adding a new state (e.g., "Recovering") is a local change (add state + transitions).
- **Positive**: State entry/exit hooks centralize side effects.
- **Positive**: Visualizable in Qt Creator's state machine viewer.
- **Negative**: State machine classes are verbose to set up (many `addTransition` calls).
- **Negative**: `QStateMachine` runs on the event loop, so state changes are asynchronous by default.