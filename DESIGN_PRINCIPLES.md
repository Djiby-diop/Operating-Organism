# OO Design Principles

This document is the short doctrine for rebuilding OO from first principles. If another document disagrees with this one, prefer this document and update the older one or mark it as historical detail.

## 1. Simplicity first

OO must be smaller than its ambition. Every subsystem must justify why it exists, what it replaces, what it costs, and how it fails.

Rules:

- Prefer one stable mechanism over many clever mechanisms.
- Delete or archive abstractions that do not have measurable inputs, outputs, invariants, and tests.
- Keep the biological metaphor as a reading aid, never as an implementation excuse.
- Choose boring contracts for the survival path.

## 2. Bare-metal first

The sovereign runtime must remain able to boot, enter a known state, and preserve its vital chain without depending on a host OS, web UI, cloud service, or lab harness.

Rules:

- Bare-metal survival code is core.
- Host tools, simulators, and yamaoo are support systems.
- A host feature may improve observability, but must not become required for survival.
- If a bare-metal feature cannot be tested on real hardware yet, it must be tiered and clearly labeled.

## 3. Survival before objectives

OO should preserve continuity before pursuing intelligence, autonomy, networking, evolution, or swarm behavior.

Rules:

- The vital chain is always more important than cortex work.
- `NORMAL`, `DEGRADED`, `SAFE`, and `RECOVERY` are operational states, not slogans.
- Reflex decisions must preempt strategic decisions when survival is at risk.
- Non-vital organs must degrade cleanly.

## 4. Reproducibility over convenience

A clever local workflow that cannot be reproduced is not part of OO's foundation.

Rules:

- There must be one command for build, one for test, and one for release.
- The dependency graph must be explicit.
- Toolchains must be pinned where possible.
- Generated artifacts must be auditable and not secretly required as source.

## 5. Governed autonomy

OO may act autonomously only inside explicit policy, ownership, logging, and recovery boundaries.

Rules:

- No uncontrolled mutation, self-modification, or network action in the core path.
- Every autonomous subsystem needs an owner, policy gate, log trail, and rollback story.
- On uncertainty, choose reversible actions.
- High-risk actions default to deny.

## 6. Every abstraction must pay rent

An organ, engine, language, repository, or document stays only if it reduces risk, improves survival, improves auditability, or enables a tested capability.

Minimum rent:

- Owner.
- Inputs and outputs.
- Invariants.
- Failure mode.
- Test or validation method.
- Reason it belongs in its tier: `core`, `optional`, `experimental`, `archive`, or `remove`.

## 7. ASCII-friendly technical base

Scripts, build files, low-level docs, and machine-parsed technical files should avoid fragile characters.

Rules:

- Use plain ASCII in scripts and build logs.
- Avoid emojis in executable or validation paths.
- Prefer stable Markdown tables and code blocks over decorative formatting.
