param(
  [switch]$NewWindow,
  [switch]$Gui,
  # If set, returns QEMU's raw exit code instead of normalizing.
  [switch]$PassThroughExitCode,

  # If set, do not normalize common non-zero QEMU exit codes.
  [switch]$NoNormalizeExitCode,

  # QEMU accelerator: auto tries WHPX (Windows Hypervisor Platform) then falls back.
  [ValidateSet('auto','whpx','tcg','none')]
  [string]$Accel = 'auto',

  # CPU model override. 'auto' keeps the existing behavior (host on WHPX, qemu64 otherwise).
  [ValidateSet('auto','host','max','qemu64')]
  [string]$Cpu = 'auto',

  # When set, request AVX2/FMA exposure in the guest CPU model (best-effort; depends on accel/model).
  [switch]$ForceAvx2,

  # Machine type (chipset). q35 can behave better on some WHPX setups.
  [ValidateSet('pc','q35')]
  [string]$Machine = 'pc',

  # Guest RAM in MB (increase for larger models)
  [ValidateRange(512, 8192)]
  [int]$MemMB = 4096,

  # Optional overrides (useful if QEMU/OVMF aren't in the default locations)
  [string]$QemuPath,
  [string]$OvmfPath,
  [string]$ImagePath
)

# Root convenience wrapper: allows running from repo root.
$script = [System.IO.Path]::Combine($PSScriptRoot, 'llm-baremetal', 'run-qemu.ps1')
if (-not (Test-Path $script)) {
  # Fallback to oo-run.ps1
  $script = [System.IO.Path]::Combine($PSScriptRoot, 'oo-run.ps1')
}
if (-not (Test-Path $script)) {
  throw "No QEMU launcher found. Expected llm-baremetal/run-qemu.ps1 or oo-run.ps1"
}

& $script @PSBoundParameters
