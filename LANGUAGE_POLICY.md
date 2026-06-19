# OO Language Policy

This policy limits language use so OO can remain understandable, reproducible, and maintainable for 20+ years.

## Principles

- The survival path must have the smallest possible toolchain surface.
- Project-owned source should converge to at least 90% C.
- The remaining project-owned source budget is at most 10%, shared only by Rust and C++ unless an architecture decision grants a temporary exception.
- A language is allowed only where its benefits exceed its long-term cost.
- No language may become a hidden dependency of the bare-metal core without an explicit architecture decision.
- Generated code must not be accepted as source unless the generator and inputs are versioned and reproducible.

## Project language budget

Target budget for project-owned source:

| Language | Target share | Role |
|---|---:|---|
| C | >= 90% | Core runtime, organs, drivers, telemetry contracts, deterministic tools |
| Rust | <= 5% | Host-side safety tooling, validators, replay/admin tools |
| C++ | <= 5% | Narrow host-side or performance tooling only when C is not enough |

The Rust/C++ split may move inside the 10% ceiling, but together they must not exceed 10% of project-owned source.

Measurement rules:

- Count project-owned source files, not archived upstream forks, vendored third-party trees, generated artifacts, model data, images, or documentation.
- Assembly is allowed only as a boot/CPU boundary exception and should remain tiny enough to audit manually.
- Python, TypeScript, PowerShell, and shell scripts are temporary support/orchestration languages, not growth languages for OO. They must not expand the core and should be reduced, generated away, or kept outside the measured survival code over time.
- Any new non-C source must explain why C is not enough and how the 90/10 budget is preserved.
- Any repository that cannot meet the budget must declare whether it is `optional`, `experimental`, or `archive/reference`.

## C

Status: primary core language.

Permitted uses:

- Freestanding bare-metal runtime code.
- Organ APIs and deterministic low-level implementations.
- Small drivers, bus contracts, state machines, and fixed-format telemetry.
- Code that must compile with minimal assumptions about the host.

Forbidden uses:

- Large unbounded frameworks in the survival path.
- Hidden dynamic allocation in vital loops.
- Business logic that belongs in host tooling or policy layers.

Rules:

- Prefer fixed-size structures for core contracts.
- Keep headers small and stable.
- Use explicit integer widths for wire/state formats.
- Any C added to the vital path needs a failure mode and validation path.

## Assembly

Status: boundary language only.

Permitted uses:

- Boot, CPU mode transitions, interrupt boundaries, ABI glue, and very small hardware primitives.

Forbidden uses:

- Business logic.
- Organ behavior.
- Policy logic.
- Optimizations that make correctness harder to audit.

Rules:

- Assembly must be isolated behind C-callable interfaces.
- Every assembly file needs a comment describing the hardware contract it owns.

## Rust

Status: bounded support language, capped inside the shared 10% Rust/C++ budget.

Permitted uses:

- Host tools and agents.
- Parsers, validators, replay tools, and safety checks where `cargo build --locked` is reproducible.
- Future freestanding modules only after the C core build/release discipline is stable.

Forbidden uses:

- Required survival-path code that depends on an unstable or undocumented toolchain.
- Pulling large dependency graphs into core runtime.
- Async/runtime frameworks in the bare-metal core.

Rules:

- Use the pinned `rust-toolchain.toml`.
- Use locked dependencies.
- Keep host-side Rust separate from bare-metal core unless an architecture decision promotes it.
- New Rust must include a budget note showing it does not push Rust+C++ above 10%.

## C++

Status: bounded support language, capped inside the shared 10% Rust/C++ budget.

Permitted uses:

- Narrow host-side tooling where C would create unnecessary complexity.
- Performance-sensitive support utilities with stable, small dependency graphs.
- Compatibility bridges when an external interface is C++ and wrapping it in C is more complex than the bridge itself.

Forbidden uses:

- Bare-metal survival-path code.
- Large frameworks, template-heavy abstractions, or hidden runtime dependencies.
- Policy logic that can be expressed as C contracts or data.

Rules:

- Prefer C ABI boundaries for any C++ component.
- Keep C++ isolated from the freestanding runtime.
- New C++ must include a budget note showing Rust+C++ remains at or below 10%.

## Python

Status: temporary/offline support only; not part of the target 90/10 project language budget.

Permitted uses:

- Model/data preparation.
- One-off analysis tools that are documented.
- Release helper scripts when they are not required by the bare-metal boot path.

Forbidden uses:

- Core runtime dependency.
- Required build step for freestanding survival code unless the exact environment is pinned.
- Hidden generation of checked-in artifacts without reproducible inputs.

Rules:

- Python scripts must state their inputs and outputs.
- Prefer PowerShell or Make for operator build orchestration already present in the repository.
- New Python must be justified as temporary tooling, not permanent runtime architecture.

## TypeScript

Status: yamaoo UI only and outside the target survival code budget.

Permitted uses:

- React/Vite frontend code for yamaoo.
- Host-side visualization, dashboards, replay, and operator interaction.

Forbidden uses:

- Bare-metal runtime.
- Survival chain decisions.
- Required release logic for the EFI/image artifact.

Rules:

- yamaoo must remain optional observability.
- UI failure must not affect OO's ability to boot or preserve the vital chain.
- New TypeScript must stay in optional UI surfaces and must not expand the survival chain.

## Build and release policy

Target operator commands:

```powershell
pwsh ./oo-build.ps1 -SkipQemu
pwsh ./tools/scripts/smoke_baremetal.ps1 -FailOnMissing -FailOnStrictMissing
wsl -e bash ./llm-baremetal/tools/scripts/make-boot-img.sh
```

The image helper is the current release/image command. It must be wrapped or extended before Phase 2 is complete so release artifacts always include checksums and provenance.

Target dependency graph:

```text
root Makefile
  -> organ Makefiles
  -> organ build/*.o
  -> liboo-all.a
  -> llm-baremetal/Makefile
  -> llama2.efi / KERNEL.EFI
  -> llm-baremetal/tools/scripts/make-boot-img.sh
  -> boot image + checksums
```

Rules:

- Build: one command must validate the core build path.
- Test: one command must validate the structural/smoke path.
- Release: one command must produce the image plus provenance artifacts. The current image helper is not enough until checksum/provenance output is mandatory.
- No hidden worktree may be required for a release.
- Host twin, yamaoo, and lab tooling may have their own builds, but they cannot be required to build the bare-metal survival chain.

## Promotion rule

To promote any language usage closer to core, the change must document:

- Why C or the existing tool is not enough.
- How the 90% C / 10% Rust+C++ budget is preserved.
- What dependency graph is added.
- How the build remains reproducible.
- How the code fails safely.
- How it will be tested on QEMU and real hardware where applicable.
