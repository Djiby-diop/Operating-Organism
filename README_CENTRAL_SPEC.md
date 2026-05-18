# README Central Specification for OO Workspace

This file specifies the target content for the future root `README.md`.
It is a specification only, created without modifying existing folder structures.

## 1) Header Block

Required:

- Project name: `OO (Operating Organism)`
- One-line mission statement:
  - "OO is a survival-first operating organism, not only an operating system."
- Short positioning paragraph (sovereign bare-metal + host twin + governed autonomy).

Optional:

- Small architecture image/logo if available.

## 2) What OO Is / Is Not

Must include two concise lists:

- What OO is:
  - long-lived organism with goals, memory, policies, and survival loop.
- What OO is not:
  - not a replacement for full desktop OS ecosystems,
  - not uncontrolled autonomy.

Reference anchors:

- `OO_VISION.md`
- `ORGANISM_MANIFEST.md`

## 3) Core Architecture at a Glance

Required:

- 7-layer high-level architecture table.
- Dual-view note:
  - biological view (organ systems),
  - engineering view (engines/modules).
- Link to supporting architecture docs.

Reference anchors:

- `oo-system/README.md`
- `OO_ORGAN_CATALOG.md`
- `OO_CONTROL_PLANES.md`

## 4) Repository Map (Current Workspace)

Required table columns:

- component,
- role,
- language,
- status,
- primary path.

Mandatory components to list:

- `llm-baremetal`
- `oo-host`
- `oo-dplus`
- `oo-sim`
- `oo-lab`
- `oo-model`
- `oo-system`

Optional section:

- external/forked references (`llm.c`, `llama2.c`, `llm-baremetal-github`) as "reference zones".

## 5) Survival/Homeostasis First

Required section:

- explain that survival invariants are evaluated before objective execution.
- mention mode transitions (`NORMAL`, `DEGRADED`, `SAFE`, `RECOVERY`).
- include pointer to invariant and flow specs.

Reference anchors:

- `OO_HOMEOSTASIS_INVARIANTS.md`
- `OO_CROSS_ORGAN_FLOWS.md`

## 6) Control Model (A+B+C Simultaneous)

Required summary:

- centralized strategic control,
- distributed organ autonomy,
- reflex/safety preemption path,
- conflict resolution principle ("survival invariants first").

Reference anchors:

- `OO_CONTROL_PLANES.md`

## 7) Quickstart (Non-Disruptive)

Required minimal command path:

- inspect status,
- run smoke path,
- run targeted validation lane.

Commands must point to existing scripts only (no new required tooling).

Reference anchors:

- `oo-system/README.md`
- `llm-baremetal/README.md`

## 8) Operator Run Path

Required:

- "first 10 minutes" operator flow:
  1) verify prerequisites,
  2) run baseline smoke,
  3) inspect vital artifacts/logs,
  4) decide go/no-go.

Do not include unstable/internal-only workflows in the first path.

## 9) Safety and Policy Notice

Required:

- policy gate statement,
- audit/journal continuity requirement,
- no ungoverned high-risk actions.

Reference anchors:

- `llm-baremetal/docs/SECURITY.md`
- policy notes in `llm-baremetal/README.md`

## 10) Documentation Index

Required grouped index:

- Vision and manifesto,
- Architecture and control,
- Runtime and integration contracts,
- Validation and recovery docs.

Use only existing files as links where possible.

## 11) Roadmap Snapshot

Required:

- short phase-based status,
- near-term milestone focused on survival/homeostasis robustness.

Reference anchors:

- `oo-system/ROADMAP.md`
- `oo-model/ROADMAP.md`
- `oo-dplus/ROADMAP.md`

## 12) Contribution Boundaries

Required:

- explain that active development folders must not be perturbed by broad refactors,
- encourage additive, minimal, auditable changes,
- mention ASCII and reproducibility discipline.

## 13) Definition of Done for README Central

The root README is considered complete when:

- a new contributor can understand OO mission in under 3 minutes,
- can identify each major module and its role,
- can run one safe smoke path without guessing,
- can find homeostasis/control specs directly,
- can identify policy/safety boundaries before running experiments.
