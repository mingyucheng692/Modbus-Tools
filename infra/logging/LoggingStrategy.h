/**
 * @file LoggingStrategy.h
 * @brief Centralized documentation of the dual-track logging strategy.
 *
 * This file serves as the single source of truth for logging conventions.
 * It contains no compiled code — only Doxygen documentation.
 *
 * Copyright (c) 2025 - present mingyucheng692
 *
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */

#pragma once

/**
 * @page logging_strategy Dual-Track Logging Strategy
 *
 * @section overview Overview
 *
 * Modbus-Tools uses a **dual-track logging architecture** with a strategic bridge:
 *
 * | Track  | Channel           | Audience           | Lifetime     | Format      |
 * |--------|-------------------|--------------------|--------------|-------------|
 * | Track A | spdlog (file)     | Developers / Debug | Persistent   | Structured  |
 * | Track B | TrafficMonitor UI | End Users          | Runtime only | Human text  |
 *
 * The **LogBridge** middleware selectively bridges TrafficEventLevel::Error
 * events from Track B into Track A for persistent fault diagnosis.
 *
 * @section trackA Track A — spdlog (Persistent File Log)
 *
 * **Applicable scenarios:**
 * - Connection lifecycle events (connect, disconnect, reconnect)
 * - I/O errors (socket write failure, serial port error)
 * - State machine transitions (idle → polling → degraded → escalated)
 * - Escalated polling errors (via LogBridge)
 * - Frame extraction anomalies (garbage bytes, buffer overflow)
 * - Modbus protocol exceptions (logged at debug level, deduplicated)
 * - Timeout / retry events
 * - Application lifecycle (init, shutdown, translation load)
 *
 * **Level semantics:**
 * | Level   | Meaning                                           |
 * |---------|---------------------------------------------------|
 * | error   | Escalated poll failure, I/O write failure         |
 * | warn    | Transient I/O errors, retries, connection drops   |
 * | info    | Lifecycle transitions, connection establishment   |
 * | debug   | Modbus exceptions (deduped), verbose diagnostics  |
 *
 * @section trackB Track B — TrafficMonitor (UI Widget)
 *
 * **Applicable scenarios:**
 * - Real-time Modbus request/response traffic display
 * - Operation feedback (manual read/write results)
 * - Poll summaries (Success: N, Error: N, Avg RTT)
 * - Non-escalated warnings (transient poll errors below threshold)
 *
 * **Events NOT written to spdlog:**
 * - TrafficEventLevel::Info  → UI only, no file log
 * - TrafficEventLevel::Warning → UI only, no file log
 * - Transient errors (below escalation threshold) → UI only
 *
 * @section bridge Bridge Rules — TrafficEvent → spdlog
 *
 * @subsection bridge_allow Events bridged to spdlog
 *
 * | Condition                              | spdlog Level | Trigger              |
 * |----------------------------------------|:------------:|----------------------|
 * | TrafficEventLevel::Error               | error        | LogBridge::relay()   |
 *
 * @subsection bridge_block Events NOT bridged
 *
 * - All TrafficEventLevel::Info events
 * - All TrafficEventLevel::Warning events
 * - TrafficEventLevel::Error events from non-poll sources (single-request errors)
 *   — these are transient and not escalated
 *
 * @section ratelimit Rate Limiting
 *
 * | Context                | Strategy                            | Window |
 * |------------------------|-------------------------------------|:------:|
 * | Modbus Exception       | Dedup by (slaveId, FC, excCode)     | 5s     |
 * | Escalated Poll Error   | Log on state change or error text change or 5s interval | 5s     |
 * | Poll Summary           | Suppressed while escalated with zero success | —      |
 *
 * @section faq FAQ
 *
 * **Q: Why aren't all TrafficEvent::Error events bridged to spdlog?**
 * A: Single-request errors (e.g., manual read to invalid address) are user
 * actions with immediate UI feedback. They don't indicate system fault and
 * would cause log flooding if persisted.
 *
 * **Q: Why is Modbus exception logged at debug level?**
 * A: A Modbus exception is a protocol-level response meaning "the device
 * received your request but cannot fulfill it". It's normal protocol behavior,
 * not a system error. Escalated poll failures are bridged at error level via
 * LogBridge, providing sufficient diagnostic context.
 *
 * **Q: How do I add a new log source?**
 * - Is it a system/IO fault? → spdlog (Track A)
 * - Is it user-facing traffic/feedback? → TrafficMonitor (Track B)
 * - Is it a persistent fault that needs file retention? → LogBridge
 */

// This file intentionally contains no code.
// See @page logging_strategy for documentation.