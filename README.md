# OO (Operating Organism)

OO is a survival-first operating organism, not only an operating system.

This repository combines a sovereign bare-metal runtime, a host twin, and governed autonomy loops with auditability, policy gates, and persistent memory.

## What OO is

- A long-lived organism with goals, memory, modes, and policies
- A bare-metal-first system with host-side orchestration
- An auditable runtime with explicit logs and recovery paths

## What OO is not

- Not a replacement for full desktop OS ecosystems
- Not uncontrolled autonomy

## Architecture at a glance

- Sovereign runtime: `llm-baremetal`
- Host twin: `oo-host`
- Language/compiler track: `oo-dplus`
- Simulation/lab layers: `oo-sim`, `oo-lab`, `oo-model`, `oo-system`

Biological view and engineering view are both documented in the architecture files listed below.

## Repository map

- `llm-baremetal`: UEFI/bare-metal runtime and reliability pipelines
- `oo-host`: host-side CLI and state orchestration
- `oo-dplus`: D+ language and compiler work
- `oo-sim`: simulation lane
- `oo-lab`: experimentation lane
- `oo-model`: model governance and evolution lane
- `oo-system`: integration and system-level contracts

## Survival first

OO evaluates survival and homeostasis invariants before objective execution.
Runtime mode transitions follow: `NORMAL -> DEGRADED -> SAFE -> RECOVERY`.

## Quickstart (safe path)

1. Inspect status docs and module readmes.
2. Run the baremetal structure smoke:

```powershell
pwsh ./tools/scripts/smoke_baremetal.ps1 -FailOnMissing -FailOnStrictMissing
```

## Pinned Toolchain

This repository pins a Rust toolchain to ensure reproducible builds across
contributors and CI. The pinned toolchain is defined in `rust-toolchain.toml`.
Use `rustup` to respect the pinned version automatically when building:

```powershell
rustup show active-toolchain
cargo build --locked
```

3. Run targeted project lanes from each module readme.

## Operator first 10 minutes

1. Verify prerequisites for your target lane.
2. Run baseline smoke checks.
3. Inspect produced logs/artifacts.
4. Decide go/no-go before deeper runs.

## Safety notice

- Policy gates apply before high-risk actions.
- Journals/artifacts must remain continuous and auditable.
- Prefer additive and reversible changes.

## Documentation index

- Vision: `OO_VISION.md`
- Manifesto: `ORGANISM_MANIFEST.md`
- Organ catalog: `OO_ORGAN_CATALOG.md`
- Control planes: `OO_CONTROL_PLANES.md`
- Cross-organ flows: `OO_CROSS_ORGAN_FLOWS.md`
- Homeostasis invariants: `OO_HOMEOSTASIS_INVARIANTS.md`
- Central spec for this readme: `README_CENTRAL_SPEC.md`

## Contribution boundaries

- Avoid broad refactors across active lanes.
- Keep changes minimal, auditable, and reproducible.
- Keep source and docs ASCII-friendly where possible.

## Debian full-stack bootstrap

Install both runtime services in one pass on Debian:

- `colony-server.service`
- `oo-host-heartbeat-watch.service`

Entry point:

- `deploy/systemd/install-oo-stack.sh`

Guide:

- `deploy/systemd/README.md`
