# Dual-Track Logging Strategy

Modbus-Tools uses a **dual-track logging architecture** with a strategic bridge:

| Track  | Channel           | Audience           | Lifetime     | Format      |
|--------|-------------------|--------------------|--------------|-------------|
| Track A | spdlog (file)     | Developers / Debug | Persistent   | Structured  |
| Track B | TrafficMonitor UI | End Users          | Runtime only | Human text  |

The **LogBridge** middleware selectively bridges Track B events into Track A
for persistent fault diagnosis. The bridge is a stateless free function
(`ui::logging::relay`), and its **only legitimate entry point** is
`TrafficLogController::publishEvent()` — producers emit signals wired to that
method; direct `relay()` calls elsewhere are forbidden.

## Track A — spdlog (Persistent File Log)

**Applicable scenarios:**
- Connection lifecycle events (connect, disconnect, reconnect)
- I/O errors (socket write failure, serial port error)
- State machine transitions (idle → polling → degraded → escalated)
- Escalated polling errors (via LogBridge)
- Frame extraction anomalies (garbage bytes, buffer overflow)
- Modbus protocol exceptions (logged at debug level, deduplicated)
- Timeout / retry events
- Application lifecycle (init, shutdown, translation load)

**Level semantics:**

| Level   | Meaning                                           |
|---------|---------------------------------------------------|
| error   | Escalated poll failure, I/O write failure         |
| warn    | Transient I/O errors, retries, connection drops   |
| info    | Lifecycle transitions, connection establishment   |
| debug   | Modbus exceptions (deduped), verbose diagnostics  |

## Track B — TrafficMonitor (UI Widget)

**Applicable scenarios:**
- Real-time Modbus request/response traffic display
- Operation feedback (manual read/write results)
- Poll summaries (Success: N, Error: N, Avg RTT)
- Non-escalated warnings (transient poll errors below threshold)

**Events NOT written to spdlog (UI only):**
- `TrafficEventLevel::Info` events that are NOT Connection lifecycle events and
  carry `traceId == 0` (e.g., poll summaries, plain status lines)

Everything else crosses the bridge — see the Bridge Rules below, which map
one-to-one onto `shouldRelay()` in `ui/logging/LogBridge.cpp`.

## Bridge Rules — TrafficEvent → spdlog

An event is bridged iff `shouldRelay()` returns true. The table below is the
authoritative contract and must stay line-aligned with the code.

### Events bridged to spdlog

| Condition                                   | spdlog Level | Trigger              |
|---------------------------------------------|:------------:|----------------------|
| `TrafficEventLevel::Warning`                | warn         | `LogBridge::relay()` |
| `TrafficEventLevel::Error`                  | error        | `LogBridge::relay()` |
| `requestType == Connection` (any level)     | per level    | `LogBridge::relay()` |
| `traceId != 0` (any level, incl. Info)      | per level    | `LogBridge::relay()` |

### Events NOT bridged

- `TrafficEventLevel::Info` events that are neither Connection lifecycle events
  nor traced (`traceId == 0`) — e.g., poll summaries and untraced status lines.
  These are high-frequency, low-diagnostic-value, and would flood the file log.

## Rate Limiting

| Context                | Strategy                            | Window |
|------------------------|-------------------------------------|:------:|
| Modbus Exception       | Dedup by (slaveId, FC, excCode)     | 5s     |
| Timeout / Retry        | Dedup by (slaveId, FC, failureKind) via `LogDedupe`; first occurrence at warn, repeats downgraded to debug | 5s     |
| Escalated Poll Error   | Log on state change or error text change or 5s interval | 5s     |
| Poll Summary           | Suppressed while escalated with zero success | —      |
| Traffic UI Error Storm | Consecutive same-summary Errors merged into one `(×N)` line per flush batch | 120ms batch |
| Traffic UI Raw Frames  | Poll frames sampled 1/N (`config::Ui::kRawFrameSampleRate`); manual frames never sampled | per frame |

## FAQ

**Q: Why are Warning and Error bridged, but untraced Info not?**
A: Warning/Error indicate faults worth persisting for diagnosis. Untraced
`Info` events (poll summaries, plain status lines) are high-frequency and
low-diagnostic-value; persisting them would flood the file log. `Info` events
that are Connection lifecycle or carry a non-zero `traceId` are still bridged,
so lifecycle and traced request flows leave no gaps in the persistent log.

**Q: Why is Modbus exception logged at debug level?**
A: A Modbus exception is a protocol-level response meaning "the device
received your request but cannot fulfill it". It's normal protocol behavior,
not a system error. Escalated poll failures are bridged at error level via
LogBridge, providing sufficient diagnostic context.

**Q: How do I add a new log source?**
- Is it a system/IO fault? → spdlog (Track A)
- Is it user-facing traffic/feedback? → TrafficMonitor (Track B)
- Does it need file retention? → route the TrafficEvent through
  `TrafficLogController::publishEvent()` (the single LogBridge entry); do NOT
  call `ui::logging::relay()` directly.
