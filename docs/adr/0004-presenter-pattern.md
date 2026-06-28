# ADR 0004: Presenter Pattern for UI-Logic Separation

**Date:** 2026-06-28

**Status:** Accepted

---

## Context

When the application was initially built, view classes (`ModbusSessionPage`, `FrameAnalyzerPage`) contained both UI rendering logic and business logic (polling orchestration, parse result formatting, settings persistence). This made unit testing difficult because every test required instantiating full Qt widgets and the application event loop.

## Decision

We adopt a lightweight **Presenter pattern** (Model-View-Presenter variant):
- **View** (Qt Widgets): responsible only for rendering, layout, and user input. No business logic.
- **Presenter** (plain `QObject`): receives user actions from the view, orchestrates domain logic, and updates the view via signals/slots or direct method calls.
- **Model** (`core` layer): domain objects (`Pdu`, `ModbusConfig`, `PollContext`) with no Qt dependency where possible.

Key presenters:
- `ModbusSessionPresenter` — manages polling lifecycle, delegates to `RequestCoordinator`, `PollingController`, `WorkerReleaseCoordinator`.
- `MainWindowPresenter` — manages navigation, toolbar state, update checking.
- `AppLifecycleCoordinator` — manages application startup/shutdown, coordinates settings and logging.

## Alternatives Considered

1. **MVVM (Qt Model/View)** — overkill for desktop widget applications; Qt's model/view is designed for item views, not widget-level separation.
2. **Passive View (strict MVP)** — view is completely passive; every interaction goes through the presenter. Too verbose for simple widgets.
3. **Presenter pattern (chosen)** — pragmatic balance; views can have simple display logic, but all business decisions are in the presenter.

## Consequences

- **Positive**: Presenters can be unit-tested with mocked views and services; no widget instantiation needed.
- **Positive**: Business logic extraction to `core` layer enables testing without Qt at all.
- **Positive**: Views can be swapped (e.g., QWidget → QML) without rewriting business logic.
- **Negative**: Presenter APIs grow as features are added; requires discipline to keep views thin.
- **Negative**: Signal/slot wiring between view and presenter adds boilerplate.