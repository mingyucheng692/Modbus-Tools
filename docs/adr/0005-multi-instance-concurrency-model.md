# ADR 0005: Multi-Instance Concurrency Model

**Date:** 2026-07-06

**Status:** Accepted

---

## Context

The application's `main.cpp` contains no single-instance lock (no `QLocalServer`, `QLockFile`, or equivalent). Users can already launch multiple application processes in parallel — this is an existing fact, not a new feature.

Despite this process-level multi-instance reality, the internal architecture was designed as if concurrent multi-protocol sessions within a single process were a hard requirement. This implicit assumption drove several layers of complexity:

- Three separate Modbus page classes (`ModbusTcpPage`, `ModbusRtuPage`, `ModbusAsciiPage`) to allow simultaneous TCP + RTU + ASCII sessions in one window.
- `AnalyzerLinkCoordinator` with a four-source `bind()` signature and a `LinkSource` enum (None/Tcp/Rtu/Ascii) to track which page feeds the shared Frame Analyzer.
- Batch settings application (`applyModbusSettingsToViews`) invoking `updateModbusSettings` on all three page pointers.
- Three concurrent `ModbusSessionPresenter` instances and their worker threads living in one process.

Review (see `abstract.md` §8) concluded that this complexity serves a need that multi-instance launching already covers: debugging multiple links at once. The architecture and the process model were in implicit contradiction.

## Decision

We adopt **"one instance, one session"** as the explicit concurrency model for Modbus-Tools.

- A single application instance hosts a single active debug session.
- Multi-protocol or multi-device debugging is achieved by launching additional application instances — the operating system process boundary is the isolation boundary.
- This decision is descriptive of current behavior (no code change is required to enable multi-instance), and prescriptive for future architecture: internal structures should not assume concurrent sessions within one process.

## Consequences

- **Positive**: The Modbus TCP/RTU/ASCII pages can be merged into a single page with in-page protocol selection (see `task.md` T9), eliminating ~190 lines of boilerplate and three near-identical subclasses.
- **Positive**: `AnalyzerLinkCoordinator` can be simplified to a single-source binding (`bind(view, analyzer)`), removing the `LinkSource` enum and its multi-source state machine.
- **Positive**: Session-layer orchestration can be simplified, since only one session lifetime needs to be managed per process.
- **Positive**: Settings isolation is naturally safe — keys are namespaced by protocol (`modbus/tcp/*`, `modbus/rtu/*`, `modbus/ascii/*`), so instances fixed to different protocols write disjoint key sets.
- **Negative**: Users who relied on keeping three concurrent sessions in one window must now launch additional instances. This is a minor UX shift; the analyzer link was already single-source, so no real workflow capability is lost.
- **Neutral**: Optional UX enhancement — a "New Window" menu action (`QProcess::startDetached`) and protocol indication in the window title may be added later for convenience.
