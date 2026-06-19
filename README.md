# OO (Operating Organism)

OO is a survival-first operating organism, not only an operating system.

OO combines a sovereign bare-metal runtime, a host twin, and governed autonomy loops. The target is a long-lived system with explicit survival invariants, persistent memory, audit trails, policy gates, and reproducible build paths.

## What OO Is

- A long-lived organism with goals, memory, modes, policies, and recovery paths.
- A bare-metal-first runtime with host-side observation and orchestration.
- A governed autonomy system where survival invariants are evaluated before objectives.
- A workspace that separates core runtime, optional support lanes, experiments, and archive/reference zones.

Reference anchors: [OO_VISION.md](OO_VISION.md), [ORGANISM_MANIFEST.md](ORGANISM_MANIFEST.md).

## What OO Is Not

- Not a replacement for full desktop OS ecosystems.
- Not uncontrolled autonomy.
- Not a web UI, model lab, or host process pretending to be the organism.
- Not a reason to keep complexity without owners, invariants, and tests.

## Core Architecture At A Glance

| Layer | Biological view | Engineering view | Primary path |
|---|---|---|---|
| 1 | Cortex | Sovereign inference/runtime shell | `llm-baremetal` |
| 2 | Kernel | Execution and scheduling boundary | `kernel-baremetal`, `oo-host` |
| 3 | Circulation | Typed event bus and flow control | `united-baremetal` |
| 4 | Memory | Working/persistent continuity | `memory-baremetal`, `oo-model` |
| 5 | Reflex/vitals | Homeostasis, safety, recovery modes | `reflex-baremetal`, `vital-baremetal` |
| 6 | Senses/interface | Telemetry, ingestion, operator bridge | `network-baremetal`, `vocal-baremetal`, `yamaoo` |
| 7 | Evolution/lab | Simulation, policy, experiments | `oo-dplus`, `oo-sim`, `oo-lab`, `oo-system` |

Dual view:

- Biological view: organs, reflexes, memory, vitals, cortex, circulation.
- Engineering view: engines, modules, contracts, control planes, invariants, artifacts.

Supporting docs: [ARCHITECTURE.md](ARCHITECTURE.md), [oo-system/README.md](oo-system/README.md), [OO_ORGAN_CATALOG.md](OO_ORGAN_CATALOG.md), [OO_CONTROL_PLANES.md](OO_CONTROL_PLANES.md).

## Repository Map

| Component | Role | Language | Status | Primary path |
|---|---|---|---|---|
| `llm-baremetal` | Sovereign UEFI/bare-metal runtime and cortex lane | C, scripts | Core | [llm-baremetal](llm-baremetal) |
| `oo-host` | Host twin, state orchestration, audit/replay support | Rust | Support/core-adjacent | [oo-host](oo-host) |
| `oo-dplus` | D+ language and policy experimentation | Rust | Experimental/support | [oo-dplus](oo-dplus) |
| `oo-sim` | Simulation and behavior test lane | C, scripts | Support | [oo-sim](oo-sim) |
| `oo-lab` | Experimentation and prototype lane | Multi | Support/incubation | [oo-lab](oo-lab) |
| `oo-model` | Offline model governance, data, export, validation | Python, Rust | Support | [oo-model](oo-model) |
| `oo-system` | Integration contracts, runtime specs, validation scripts | C, Python, PowerShell | Canonical support | [oo-system](oo-system) |

Reference zones:

- `llm.c`, `llama2.c`, and `llm-baremetal-github` style forks are reference or upstream zones, not automatic core dependencies.
- `yamaoo` is a host-side interface/observability lane, not a required bare-metal dependency.

## Language Direction

OO is C-first. Project-owned source should converge to at least 90% C. The remaining 10% is reserved for bounded Rust and C++ support code.

Python, TypeScript, PowerShell, and shell are support/orchestration languages only. They must not become survival-chain growth languages.

Full rules: [LANGUAGE_POLICY.md](LANGUAGE_POLICY.md).

## Survival And Homeostasis First

OO evaluates survival invariants before objective execution. The vital path must continue even when cortex work, host tooling, UI, network, or experiments degrade.

Runtime modes:

- `NORMAL`: invariants green; objectives may run inside policy.
- `DEGRADED`: non-vital failure or pressure; throttle optional work and preserve continuity.
- `SAFE`: vital risk or policy uncertainty; deny high-risk actions and keep only safe subsets.
- `RECOVERY`: restore state, replay journals, verify invariants, then return to `NORMAL` only when safe.

Invariant and flow specs: [OO_HOMEOSTASIS_INVARIANTS.md](OO_HOMEOSTASIS_INVARIANTS.md), [OO_CROSS_ORGAN_FLOWS.md](OO_CROSS_ORGAN_FLOWS.md).

## Control Model

OO uses A+B+C control simultaneously:

- Centralized strategic control keeps mission direction, global goals, mode transitions, and policy posture coherent.
- Distributed organ autonomy lets organs perform local health checks, scheduling, and fallback inside global policy.
- Reflex/safety preemption handles threshold breaches before strategic planning completes.
- Conflict resolution principle: survival invariants first, hard policy second, optimization last.

Details: [OO_CONTROL_PLANES.md](OO_CONTROL_PLANES.md).

## Quickstart

Non-disruptive path using scripts that exist in this workspace:

```powershell
# inspect current repository status
git status --short

# run the baseline bare-metal structure smoke
pwsh ./tools/scripts/smoke_baremetal.ps1 -FailOnMissing -FailOnStrictMissing

# run targeted runtime validation when working in oo-system
pwsh -NoProfile -Command "Push-Location ./oo-system; ./scripts/runtime-v1-smoke.ps1; Pop-Location"
```

Optional build path:

```powershell
pwsh ./oo-build.ps1 -SkipQemu
```

Pinned Rust toolchain:

```powershell
rustup show active-toolchain
cargo build --locked
```

Module-specific paths are documented in [oo-system/README.md](oo-system/README.md) and [llm-baremetal/README.md](llm-baremetal/README.md).

## Operator First 10 Minutes

1. Verify prerequisites for the lane you are touching: PowerShell, WSL/make/gcc where needed, pinned Rust when using Rust code.
2. Run baseline smoke: `pwsh ./tools/scripts/smoke_baremetal.ps1 -FailOnMissing -FailOnStrictMissing`.
3. Inspect vital artifacts/logs: `OO_UART.log`, `artifacts/`, module build output, and runtime validation reports.
4. Decide go/no-go before deeper runs such as QEMU, image creation, host twin replay, or yamaoo.

Do not put unstable or internal-only workflows in the first operator path.

## Safety And Policy Notice

- Policy gates apply before high-risk actions.
- Audit journals and artifacts must remain continuous enough to reconstruct decisions.
- No ungoverned high-risk actions, uncontrolled mutation, hidden network autonomy, or silent policy bypass.
- If policy is unavailable for a critical path, default to deny and move toward `SAFE`.

References: [llm-baremetal/docs/SECURITY.md](llm-baremetal/docs/SECURITY.md), [llm-baremetal/README.md](llm-baremetal/README.md).

## Documentation Index

Vision and manifesto:

- [OO_VISION.md](OO_VISION.md)
- [ORGANISM_MANIFEST.md](ORGANISM_MANIFEST.md)
- [MANIFESTO_OO.md](MANIFESTO_OO.md)

Architecture and control:

- [ARCHITECTURE.md](ARCHITECTURE.md)
- [DESIGN_PRINCIPLES.md](DESIGN_PRINCIPLES.md)
- [OO_ORGAN_CATALOG.md](OO_ORGAN_CATALOG.md)
- [OO_CONTROL_PLANES.md](OO_CONTROL_PLANES.md)
- [OO_HOMEOSTASIS_INVARIANTS.md](OO_HOMEOSTASIS_INVARIANTS.md)
- [OO_CROSS_ORGAN_FLOWS.md](OO_CROSS_ORGAN_FLOWS.md)

Runtime and integration contracts:

- [oo-system/README.md](oo-system/README.md)
- [oo-system/docs/OO_RUNTIME_V1_BLUEPRINT.md](oo-system/docs/OO_RUNTIME_V1_BLUEPRINT.md)
- [oo-system/docs/OO_RUNTIME_V1_ORGAN_HOST_CONTRACT.md](oo-system/docs/OO_RUNTIME_V1_ORGAN_HOST_CONTRACT.md)
- [oo-system/docs/OO_EVENT_CONTRACT.md](oo-system/docs/OO_EVENT_CONTRACT.md)
- [llm-baremetal/docs/OO_SOMAMIND_RUNTIME_CONTRACT.md](llm-baremetal/docs/OO_SOMAMIND_RUNTIME_CONTRACT.md)

Validation and recovery:

- [oo-system/docs/OO_RUNTIME_V1_REMEDIATION_PLAYBOOK.md](oo-system/docs/OO_RUNTIME_V1_REMEDIATION_PLAYBOOK.md)
- [oo-system/docs/OO_RUNTIME_V1_REASON_CODES.md](oo-system/docs/OO_RUNTIME_V1_REASON_CODES.md)
- [oo-system/scripts/runtime-v1-smoke.ps1](oo-system/scripts/runtime-v1-smoke.ps1)
- [tools/scripts/smoke_baremetal.ps1](tools/scripts/smoke_baremetal.ps1)

Governance:

- [LANGUAGE_POLICY.md](LANGUAGE_POLICY.md)
- [CONTRIBUTING.md](CONTRIBUTING.md)
- [ROADMAP.md](ROADMAP.md)
- [README_CENTRAL_SPEC.md](README_CENTRAL_SPEC.md)

## Roadmap Snapshot

Phase status:

- Phase 0: doctrine freeze and module classification is documented in [ARCHITECTURE.md](ARCHITECTURE.md).
- Phase 1: Minimal Viable OO focuses on deterministic boot, core organs, survival modes, memory journal, reflex preemption, and one telemetry path.
- Phase 2: deterministic build/test/release must remove hidden dependencies and produce reproducible artifacts.
- Phase 3: survival mode validation must prove `NORMAL`, `DEGRADED`, `SAFE`, and `RECOVERY` under fault injection.
- Phase 4+: host twin, yamaoo observability, controlled evolution, dream/swarm/model governance remain outside the vital proof until measured.

Near-term milestone: make survival/homeostasis robustness testable before expanding autonomy.

References: [ROADMAP.md](ROADMAP.md), [oo-system/ROADMAP.md](oo-system/ROADMAP.md), [oo-model/ROADMAP.md](oo-model/ROADMAP.md), [llm-baremetal/oo-dplus/ROADMAP.md](llm-baremetal/oo-dplus/ROADMAP.md).

## Contribution Boundaries

- Do not perturb active development folders with broad refactors.
- Prefer additive, minimal, auditable changes.
- Keep reproducibility visible: commands, inputs, outputs, checksums, and generated artifacts must be explainable.
- Keep scripts and low-level technical docs ASCII-friendly.
- New organs, languages, or repos must identify owner, inputs, outputs, invariants, failure mode, tests, and tier.

See [CONTRIBUTING.md](CONTRIBUTING.md).

## Definition Of Done For This README

The central README is complete when a new contributor can:

- Understand OO's mission in under 3 minutes.
- Identify each major module and its role.
- Run one safe smoke path without guessing.
- Find homeostasis and control specs directly.
- Identify policy and safety boundaries before running experiments.

## Debian Full-Stack Bootstrap

Install both runtime services in one pass on Debian:

- `colony-server.service`
- `oo-host-heartbeat-watch.service`

Entry point: [deploy/systemd/install-oo-stack.sh](deploy/systemd/install-oo-stack.sh)

Guide: [deploy/systemd/README.md](deploy/systemd/README.md)
