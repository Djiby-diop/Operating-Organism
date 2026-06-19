# OO Roadmap

This roadmap moves OO from first-principles doctrine to a Minimal Viable OO, then to long-term controlled evolution.

## Phase 0: Doctrine freeze and classification

Goal: stop architectural drift before more implementation work accumulates.

Scope:

- Make `ARCHITECTURE.md` the canonical short source of truth.
- Make `DESIGN_PRINCIPLES.md`, `LANGUAGE_POLICY.md`, and `CONTRIBUTING.md` required governance docs.
- Classify each module as `core`, `optional`, `experimental`, `archive`, or `remove`.
- Mark older contradictory docs as detailed annexes or historical references.

Modules:

- Core: `kernel-baremetal`, `united-baremetal`, `memory-baremetal`, `reflex-baremetal`, `vital-baremetal`, `identity-baremetal`, `network-baremetal`, `vocal-baremetal`, `llm-baremetal`.
- Optional: `sense-baremetal`, `proprioception-baremetal`, `regen-baremetal`, `shadow-baremetal`.
- Experimental: `dream-baremetal`, `evolution-baremetal`, `swarm-baremetal`, `bot-baremetal` until merged into a clear `IntegrityGuard` contract or removed.
- Support/optional: `oo-host`, `oo-system`, `oo-sim`, `oo-model`, `control-planes`, `yamaoo`.
- Support/experimental: `oo-lab`, `oo-dplus`.
- Archive/reference: `llm.c`, `llama2.c`, duplicate worktrees, historical forks unless explicitly promoted.

Acceptance criteria:

- A contributor can read `README.md`, `ARCHITECTURE.md`, and `DESIGN_PRINCIPLES.md` and explain OO's core in under 10 minutes.
- Every module has one tier.
- Every core module has owner role, inputs, outputs, invariant, and failure mode.
- Every non-core module has an explicit reason it is optional, experimental, or archive/reference.

## Phase 1: Minimal Viable OO

Goal: prove the smallest survival-first organism.

Minimal Viable OO must:

1. Boot UEFI into `llm-baremetal` or the runtime shell.
2. Initialize core organs deterministically.
3. Circulate typed events through `united-baremetal`.
4. Enforce `NORMAL`, `DEGRADED`, `SAFE`, and `RECOVERY` through `vital-baremetal`.
5. Allow `reflex-baremetal` to preempt non-vital work.
6. Record minimal state through `memory-baremetal`.
7. Expose telemetry through one non-blocking path.
8. Build with one documented command.

Deferred from MVO:

- Swarm/colony behavior.
- Dream/replay autonomy.
- Evolution/mutation workflow.
- Advanced model governance.
- Full production network stack.
- Rich yamaoo UI behavior beyond observability.

Acceptance criteria:

- `pwsh ./oo-build.ps1 -SkipQemu` validates the core path.
- The vital chain survives failure of a non-vital organ.
- Telemetry failure does not block survival.

## Phase 2: Deterministic build, test, and release

Goal: make the project reproducible enough to maintain for 20+ years.

Scope:

- Root `Makefile` produces organ objects and `liboo-all.a`.
- `llm-baremetal/Makefile` consumes the unified archive.
- Image creation is documented through `llm-baremetal/tools/scripts/make-boot-img.sh`.
- A future release orchestrator may wrap image creation, but it must not hide the source dependency graph.
- Release artifacts include checksums and provenance.

Target commands:

```powershell
pwsh ./oo-build.ps1 -SkipQemu
pwsh ./tools/scripts/smoke_baremetal.ps1 -FailOnMissing -FailOnStrictMissing
wsl -e bash ./llm-baremetal/tools/scripts/make-boot-img.sh
```

Acceptance criteria:

- Build graph is explicit.
- No hidden worktree is required.
- A clean environment can reproduce the release image from documented inputs.

## Phase 3: Survival mode validation

Goal: prove that homeostasis is operational, not only documented.

Scope:

- Validate global invariants from `OO_HOMEOSTASIS_INVARIANTS.md`.
- Test transitions:
  - `NORMAL -> DEGRADED` for non-vital failure or sustained pressure.
  - `DEGRADED -> SAFE` for vital risk or policy gate failure.
  - `SAFE -> RECOVERY` when recovery prerequisites are available.
  - `RECOVERY -> NORMAL` only after invariants are restored.
- Validate reflex preemption before strategic planning.

Acceptance criteria:

- Fault injection demonstrates each mode transition.
- Reflex path remains bounded.
- Recovery actions are reversible or explicitly safe-by-default.

## Phase 4: Host twin and yamaoo observability

Goal: make the organism visible without making the UI a survival dependency.

Scope:

- `oo-host` receives, stores, replays, and audits runtime telemetry.
- yamaoo visualizes vitals, organ states, cortex thoughts, and alerts.
- Simulators and replay tools can drive yamaoo when hardware is absent.

Rules:

- yamaoo is optional.
- WebSocket or UI failure must not affect the bare-metal runtime.
- Host commands must be auditable.

Acceptance criteria:

- yamaoo can show live or replayed telemetry.
- Bare-metal survival tests pass with yamaoo stopped.
- Operator can distinguish real hardware, QEMU, and replay sources.

## Phase 5: Controlled evolution and long-term maintenance

Goal: allow OO to grow without collapsing under complexity.

Scope:

- Promote experimental organs only after they satisfy architecture decision rules.
- Define mutation governance before enabling evolution-like behavior.
- Keep `dream`, `swarm`, advanced networking, and model governance outside the core until proven.
- Periodically archive dead scripts, duplicate docs, and abandoned lanes.

Promotion gates:

- Owner assigned.
- Invariants and failure modes documented.
- Tests pass.
- Build/release remains deterministic.
- Complexity budget is paid.

Acceptance criteria:

- No experimental capability can affect the survival path without review.
- Archive remains off the build path.
- Core stays explainable and auditable.

## Migration path from current repository

Immediate actions:

1. Treat `ARCHITECTURE.md` as canonical.
2. Keep `README.md` as the operator entry point and link the canonical docs from it.
3. Keep older OO docs as annexes only when they add detail without contradiction.
4. Move or mark duplicate scripts/docs as archive after confirming no build references remain.
5. Keep yamaoo in the host-twin lane.
6. Keep experimental organs out of MVO until their contracts are measurable.

Ready for implementation when:

- The MVO core module list is accepted.
- Build/test/release commands are stable.
- Organ contracts are documented.
- The next implementation task can target one narrow subsystem without reopening the whole architecture.
