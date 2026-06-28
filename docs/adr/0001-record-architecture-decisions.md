# ADR 0001: Record Architecture Decisions

**Date:** 2026-06-28

**Status:** Accepted

---

## Context

The Modbus-Tools project has grown through multiple iterative refactoring phases (P0-P2). Architectural decisions — such as state machine patterns, exception policy, and presenter-view separation — were made during development but never formally documented. This makes onboarding and future maintenance harder.

## Decision

We adopt lightweight Architecture Decision Records (ADRs) as our documentation format, following the [Michael Nygard template](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions).

Each ADR documents one significant architectural decision, including:
- **Context**: the forces at play (technical, business, social)
- **Decision**: the chosen approach
- **Consequences**: what becomes easier / harder as a result

ADRs are stored in `docs/adr/`, numbered sequentially with a short title. Superseded ADRs are marked as such but never deleted.

## Consequences

- **Positive**: New contributors can understand the rationale behind key design choices.
- **Positive**: Decisions are reviewable and reversible with clear traceability.
- **Negative**: Adds maintenance overhead — ADRs must be updated when decisions change.