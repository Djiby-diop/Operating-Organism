# Contributing to OO

OO is a survival-first operating organism. Contributions must make the system simpler, more reproducible, more auditable, or measurably more survivable.

## Contribution principles

- Preserve the bare-metal survival chain first.
- Keep changes small, documented, and reversible.
- Do not add an abstraction unless it pays rent.
- Do not make host-side tools required for bare-metal survival.
- Keep scripts, build files, and low-level docs ASCII-friendly.

## Ownership model

Every subsystem must have an owner role. A person or team may fill multiple roles, but the role must be explicit in design and review.

| Area | Owner role | Required review focus |
|---|---|---|
| `kernel-baremetal` | Runtime execution owner | boot, scheduling, failure containment |
| `united-baremetal` | Circulation owner | event contracts, backpressure, ordering |
| `memory-baremetal` | Continuity owner | persistence, journal integrity, rollback |
| `reflex-baremetal` | Survival owner | reflex latency, preemption, safe fallback |
| `vital-baremetal` | Homeostasis owner | mode transitions, invariants, recovery |
| `identity-baremetal` | Identity owner | stable identity, hashes, trust anchors |
| `network-baremetal` / `vocal-baremetal` | Telemetry owner | non-blocking communication, fallback paths |
| `llm-baremetal` | Cortex owner | graceful degradation, policy gates |
| `oo-host` / `yamaoo` | Host twin owner | optional observability, replay, administration |
| `oo-sim` / `oo-lab` | Lab owner | fault injection, reproducible experiments |
| `oo-model` | Model governance owner | provenance, validation, offline reproducibility |

## Required checks before merge

For documentation-only changes:

- `README.md`, `ARCHITECTURE.md`, `DESIGN_PRINCIPLES.md`, `LANGUAGE_POLICY.md`, `CONTRIBUTING.md`, and `ROADMAP.md` must not contradict each other.
- Any changed doctrine must identify what older document becomes historical detail.

For bare-metal core changes:

```powershell
pwsh ./oo-build.ps1 -SkipQemu
pwsh ./tools/scripts/smoke_baremetal.ps1 -FailOnMissing -FailOnStrictMissing
```

For release/image changes:

```powershell
wsl -e bash ./llm-baremetal/tools/scripts/make-boot-img.sh
```

For host/yamaoo changes:

- Backend and frontend checks are required for the changed lane.
- UI failure must not block the bare-metal survival chain.

## Architecture decision rule

Any major change must document:

- Why it exists.
- What simpler alternative was rejected.
- What it replaces or removes.
- Which tier it belongs to: `core`, `optional`, `experimental`, `archive`, or `remove`.
- Which owner role is responsible.
- Inputs, outputs, invariants, and failure mode.
- How it is tested.
- How it fails safely.

Use a short Markdown note under the appropriate docs or architecture decision location when one exists. Do not create a large process before the core stabilizes.

## Complexity budget

Adding complexity requires paying for it.

Every non-trivial addition must do at least one of the following:

- Remove an older subsystem, script, dependency, or duplicate document.
- Merge overlapping concepts.
- Make a build/test/release path more reproducible.
- Add a measurable survival invariant or validation.
- Clearly isolate an experiment outside the core path.

Rejected patterns:

- New organ without owner and tests.
- New language without `LANGUAGE_POLICY.md` justification.
- New build script that duplicates existing build commands.
- New host dependency required by the bare-metal survival path.
- New biological concept without engineering contract.

## Repository strategy

### Public

Public repositories contain the minimal auditable core, canonical docs, reproducible build scripts, and non-sensitive test fixtures.

Allowed:

- Bare-metal core source.
- Canonical documentation.
- Reproducible build/test/release scripts.
- Public model fixtures or checksums.

Forbidden:

- Secrets.
- Hardware-specific private credentials.
- Undocumented generated artifacts required as source.

### Private

Private repositories contain sensitive material that must not be required to build or understand the public core.

Allowed:

- Lab hardware notes.
- Sensitive deployment details.
- Private datasets or keys.

Rule: private material may improve operation, but cannot be required for public survival builds.

### Incubation

Incubation repositories or directories contain experiments that are not yet core.

Allowed:

- `dream`, `swarm`, `evolution`, advanced drivers, model governance experiments.
- Alternative language/toolchain experiments.

Exit criteria:

- Owner assigned.
- Inputs/outputs/invariants/failure mode documented.
- Tests or validation exist.
- Dependency impact is known.
- A removal or simplification offsets the complexity.

Promotion from incubation to core requires updating `ARCHITECTURE.md` first. The promotion must include the exact tier change, owner role, input/output contract, invariant, failure mode, validation command, and rollback path.

### Archive

Archive locations contain historical copies, replaced docs, abandoned scripts, and worktrees kept only for reference.

Rules:

- Archive is read-only reference.
- Archive must not be on the build path.
- If a file is needed again, restore it through review instead of depending on it implicitly.

Archive/reference code such as upstream forks must not be silently imported by scripts. If a script needs archived code, the dependency must be converted into an explicit supported path or removed.

## Documentation requirements

Update documentation when a change modifies:

- Organ classification.
- Build/test/release commands.
- Language boundaries.
- Survival modes or invariants.
- Repository tiering.
- yamaoo or host-twin dependency boundaries.

Keep docs short enough to maintain. Prefer updating canonical docs over adding another manifesto.
