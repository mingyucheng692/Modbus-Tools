# Architecture Decision Records (ADRs)

| ID | Decision | Core Mechanism | Trade-off / Invariant |
|:---|:---|:---|:---|
| **0001** | **Disable C++ Exceptions** | All first-party targets (`core`, `infra`, `ui`) compile with `-fno-exceptions` (`/EHs-c-`, `_HAS_EXCEPTIONS=0`). Errors propagate via `common::Result<T, Enum>` with `[[nodiscard]]`. | Disallows throwing STL APIs. OOM or unvalidated `.value()` calls terminate via `std::abort`. Tests exempt for GTest. |
| **0002** | **Dual State Machine** | Core session/request lifecycles use CRTP `StateMachineBase` with `constexpr` tables and lock-free CAS (`std::atomic`). UI maintains no parallel FSM; state is purely derived via `ModbusSessionPresenter::deriveUiState()`. | Core transitions are allocation-free and thread-safe. Table edits require compile-time array updates. |
| **0003** | **MVP & Worker Lifecycle** | View (passive widget) and Presenter (`QObject`) decoupled from non-GUI `core`. Background tasks use `moveToThread`. Teardown executes via `WorkerReleaseCoordinator` 4-stage bounded sequence (3s force fallback). | Eliminates socket UAF / cross-thread `sendEvent` asserts. Requires queued signal/slot wiring boilerplate. |
| **0004** | **Process-Level Concurrency** | "One Instance, One Session". Multiple concurrent links debugged via separate OS processes, eliminating multi-session multiplexing and consolidating protocol views into unified `ModbusPage`. | Leverages OS process memory isolation. Concurrent instances sharing `config.ini` exhibit last-write-wins on exit. |

---

## Architectural Invariants

1. **Zero-GUI Domain Core**: `core/` MUST NOT depend on `QtWidgets` or GUI event loop primitives.
2. **Deterministic Teardown**: Sockets and serial ports MUST transition to `Closed` on their owning I/O thread before thread termination; no cross-thread blocking deletions.
3. **Single Truth Source**: UI layer MUST NOT maintain independent connection state; all connection state is read or derived from Core atomic FSMs.
4. **Exe-Dir Portability**: All writable data (config, logs, staging) MUST resolve next to the binary via `PathResolver`; `QStandardPaths` writable locations are forbidden.
