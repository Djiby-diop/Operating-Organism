# OO Control Planes

This file formalizes the three simultaneous control modes for OO:

- Centralized strategic control (A)
- Distributed organ autonomy (B)
- Hybrid local reflexes (C)

## 1) Centralized Strategic Plane (Brain-Like)

## Purpose

- Keep a coherent long-horizon direction.
- Maintain global goals, mission priorities, and policy posture.
- Decide system-wide mode transitions (`NORMAL`, `DEGRADED`, `SAFE`, `RECOVERY`).

## Responsibilities

- Global planning and reprioritization.
- Cross-organ arbitration when intents conflict.
- Budgeting of compute/time/risk envelopes.
- Approval requirements for high-impact actions.

## Main Engines

- `StrategicBrainEngine`
- `GlobalGoalEngine`
- `ModeGovernorEngine`
- `CrossOrganArbitrationEngine`

## Existing Anchors

- `oo-host` for runtime state, goals, journal-driven steering.
- `oo-system` for system-level orchestration and CLI control surface.

## 2) Distributed Organ Autonomy Plane

## Purpose

- Let each organ remain useful without waiting for central decisions.
- Improve resilience and throughput under partial failure.
- Support specialized behavior per organ.

## Responsibilities

- Local scheduling and execution.
- Local health checks and local fallback.
- Local optimization inside global policy bounds.
- Local event publication to shared bus.

## Main Engines

- `OrganAutonomyEngine`
- `LocalSchedulerEngine`
- `LocalHealthEngine`
- `OrganEventPublisher`

## Existing Anchors

- `llm-baremetal` for sovereign local loops.
- `oo-sim` and `oo-lab` for local behavior experimentation.
- `oo-dplus` for policy gates used by autonomous actions.

## 3) Hybrid Reflex/Safety Plane

## Purpose

- Guarantee survival-first responses at very low latency.
- Execute corrections before strategic loop completes.
- Prevent fatal cascades.

## Responsibilities

- Reflex action on threshold breach.
- Emergency mode lowering and isolation.
- Safe fallback action execution.
- Escalation signal to strategic plane after immediate stabilization.

## Main Engines

- `ReflexEngine`
- `SafetyActionEngine`
- `IsolationEngine`
- `EscalationEngine`

## Existing Anchors

- `llm-baremetal` for close-to-metal reflex paths.
- `oo-dplus` for hard policy enforcement and safety constraints.

## 4) Plane Priority Rules

Priority is dynamic, but survival dominates:

1. Reflex/Safety Plane (immediate, life-preserving)
2. Centralized Strategic Plane (global coherence)
3. Distributed Organ Plane (continuous operation)

Under stable conditions:

- Distributed plane runs continuously.
- Strategic plane periodically adjusts priorities.
- Reflex plane stays armed, mostly inactive.

Under unstable conditions:

- Reflex plane can preempt all others.
- Strategic plane can freeze selected distributed activities.
- Distributed plane continues only in safe subsets.

## 5) Conflict Resolution Contract

When planes disagree:

- Rule 1: Survival invariants win.
- Rule 2: Hard policy (`oo-dplus`) wins over optimization.
- Rule 3: Strategic goal wins only if invariants remain green.
- Rule 4: If uncertainty is high, choose reversible action.

## 6) Control Tick Model

Each global tick follows:

1. Read vital signals and policy state.
2. Execute mandatory reflexes if needed.
3. Evaluate strategic priorities and mode.
4. Dispatch bounded local tasks to organs.
5. Collect outcomes and update memory/journal.

## 7) Observability Requirements

Each plane must output:

- current state,
- last decision,
- reason code,
- confidence/risk score,
- next recommended action.
