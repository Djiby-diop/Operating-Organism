# OO Homeostasis Invariants

This file defines survival/homeostasis invariants for OO.
Each invariant is measurable and has thresholded actions.

## Severity Levels

- `INFO`: nominal variation, no action required.
- `WARN`: deviation detected, correction recommended.
- `CRITICAL`: immediate correction required.
- `FATAL`: emergency fallback required to preserve continuity.

## 10 Global Invariants

## I1. Control Loop Liveness

- Metric: maximum time between control ticks.
- WARN: tick delay > soft threshold.
- CRITICAL: tick delay > hard threshold.
- Action: degrade non-vital workloads, force reflex loop.

## I2. State Integrity

- Metric: state checksum/hash validity, schema validity.
- WARN: recoverable mismatch in non-vital fields.
- CRITICAL: mismatch in vital fields.
- Action: load recovery snapshot, lock risky actions.

## I3. Journal Continuity

- Metric: append-only journal progress without gaps.
- WARN: delayed writes or missing optional events.
- CRITICAL: missing vital event window.
- Action: force sync of vital events, emit continuity alarm.

## I4. Policy Gate Health

- Metric: policy evaluator availability and consistency.
- WARN: delayed policy response.
- CRITICAL: policy unavailable for critical action path.
- Action: deny high-risk actions by default, SAFE mode.

## I5. Resource Circulation Balance

- Metric: CPU/memory/IO pressure budget within safe envelope.
- WARN: sustained pressure beyond nominal.
- CRITICAL: pressure threatens loop liveness.
- Action: throttle adaptive organs, keep vital chain only.

## I6. Memory Continuity

- Metric: ability to read/write short-term and persistent memory.
- WARN: short-term degradation.
- CRITICAL: persistent layer unreachable or corrupted.
- Action: isolate memory writers, fallback to minimal persistence.

## I7. Organ Isolation Integrity

- Metric: fault containment effectiveness across organ boundaries.
- WARN: non-fatal cross-organ contamination.
- CRITICAL: propagation into vital chain.
- Action: quarantine affected organ, activate substitute path.

## I8. Reflex Path Readiness

- Metric: reflex action path execution time and success rate.
- WARN: increased reflex latency.
- CRITICAL: reflex path cannot execute safety action.
- Action: force emergency profile, stop non-vital planning.

## I9. Recovery Capability

- Metric: recovery snapshot freshness and replay success.
- WARN: stale snapshot beyond target window.
- CRITICAL: latest snapshot unusable.
- Action: create forced minimal snapshot, reduce mutation pace.

## I10. Trust Boundary Integrity

- Metric: validation status of external inputs/artifacts.
- WARN: malformed but non-critical input.
- CRITICAL: suspicious input touching critical path.
- Action: drop/blacklist input route, escalate immune alert.

## Threshold Model

Each invariant uses:

- `soft_threshold`: first warning boundary.
- `hard_threshold`: critical boundary.
- `panic_threshold`: immediate emergency fallback.

Threshold values are environment-specific (`qemu`, `real-hw`, `lab`) and must be configured outside this document.

## Standard Corrective Actions

- `A1`: Throttle non-vital organs.
- `A2`: Switch to `DEGRADED` mode.
- `A3`: Switch to `SAFE` mode.
- `A4`: Quarantine failing organ.
- `A5`: Restore recovery snapshot.
- `A6`: Activate reflex-only vital chain.

## Escalation Rules

- Single WARN: local correction in distributed plane.
- Repeated WARN: strategic plane reprioritization.
- Any CRITICAL on vital invariant: reflex preemption + SAFE/DEGRADED gate.
- Any FATAL on continuity invariant (`I1`, `I2`, `I6`, `I8`): emergency survival profile.
