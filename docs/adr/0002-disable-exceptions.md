# ADR 0002: Disable C++ Exceptions in First-Party Code

**Date:** 2026-06-28

**Status:** Accepted

---

## Context

Modbus-Tools is a desktop application that communicates with hardware over serial and TCP. The domain is deterministic: protocol parsing, CRC calculation, and state transitions. Exceptions add non-deterministic control flow, make reasoning about error paths harder, and increase binary size.

Qt itself does not throw exceptions by design. SPDLOG (third-party logging) supports an exception-free mode via `SPDLOG_NO_EXCEPTIONS`.

## Decision

We compile all first-party targets (`core`, `infra`, `ui`) with `-fno-exceptions` (GCC/Clang) or `/EHs-c-` (MSVC). We set `SPDLOG_NO_EXCEPTIONS=ON` for spdlog.

Error handling uses `std::optional`, result types with error strings, and return-code checks instead of try/catch.

## Alternatives Considered

1. **Enable exceptions everywhere** — simpler error propagation but contradicts Qt's contract and bloats the binary.
2. **Exceptions only in `core`** — inconsistent; callers would still need to be exception-aware.
3. **No exceptions (chosen)** — consistent, predictable, aligns with Qt's design.

## Consequences

- **Positive**: Smaller binary, predictable control flow, no hidden exception paths.
- **Positive**: Aligns with Qt's no-exception philosophy.
- **Negative**: Cannot use standard library APIs that throw (e.g., `std::stoi`). Must use `QMutexLocker` instead of `std::lock_guard` in exception-free code.
- **Negative**: Error propagation requires explicit checking, which can be verbose.