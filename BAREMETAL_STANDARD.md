# Baremetal Module Standard (v0.1)

This standard defines a minimal contract for every `*-baremetal` module in this repository.

## 1) Required Structure

Each module should contain:

- `README.md`: module purpose, build command, test command, key interfaces
- `include/`: public headers and exported interfaces
- `src/`: implementation sources

## 2) Optional But Recommended

- `Makefile`: one-command local build entrypoint
- `tests/`: smoke/unit tests for host-side validation
- `docs/`: protocol notes, API details, architecture notes

## 3) README Minimum Sections

Each `README.md` should include:

1. Scope: what this module does and does not do
2. Dependencies: required headers/modules/tools
3. Build: exact local command(s)
4. Test/Smoke: exact local command(s)
5. Integration: where it plugs into the organism stack

## 4) Build Contract (recommended)

When a `Makefile` exists, provide at least:

- `make build`
- `make test`
- `make clean`

If those targets are not available yet, document equivalent commands in `README.md`.

## 5) Smoke Validation Policy

Root-level smoke checks should validate, at minimum:

- module exists and is accessible
- `README.md` present
- `include/` present
- `src/` present
- optional `Makefile` presence for build automation tracking

The smoke script exposes two views:

- strict view: enforces `README.md` + `include/` + `src/`
- compatibility view: accepts legacy layouts with `core/` and/or `bridge/`
- compatibility view also accepts header-only modules (`README.md` + `include/`)

This allows incremental harmonization without blocking legacy modules.

## 6) Current Target Modules

- `bot-baremetal`
- `dream-baremetal`
- `evolution-baremetal`
- `identity-baremetal`
- `kernel-baremetal`
- `memory-baremetal`
- `network-baremetal`
- `reflex-baremetal`
- `sense-baremetal`
- `shadow-baremetal`
- `swarm-baremetal`
- `united-baremetal`

## 7) Adoption Strategy

1. Run the smoke script from repository root.
2. Fix missing `README.md`, `include/`, or `src/` first.
3. Add or normalize build commands next.
4. Promote module status to CI once smoke-stable.

## 8) CI Automation

Repository workflow:

- `.github/workflows/baremetal-structure-smoke.yml`

CI behavior:

- runs compatibility smoke as blocking gate (`-FailOnMissing`)
- runs strict gate only on dedicated branches named `strict-baremetal/*`
- emits strict/compat JSON + Markdown report artifacts for progressive harmonization

Report generator:

- `tools/scripts/render_baremetal_report.ps1`
