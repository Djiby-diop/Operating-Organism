param(
  [Nullable[bool]]$Headless,
  # Forwarded to llm-baremetal/test-qemu.ps1
  [switch]$Interactive,
  [ValidateSet('auto','whpx','tcg','none')]
  [string]$Accel = 'tcg',
  [ValidateRange(512, 8192)]
  [int]$MemMB = 4096,
  # 0 disables the watchdog timeout (wait indefinitely for REPL needle).
  [ValidateRange(0, 86400)]
  [int]$TimeoutSec = 0,
  [string]$ModelBin = 'stories110M.bin',
  [switch]$SkipBuild,
  # If set, skips oo-guard prebuild check during image rebuild (dev convenience).
  [switch]$SkipPrebuild,
  [string]$QemuPath,
  [string]$OvmfPath,
  [string]$ImagePath,
  [switch]$ForceAvx2
)

$ErrorActionPreference = 'Stop'

# Touch $Headless so analyzers treat it as used (it is forwarded via @PSBoundParameters).
if ($PSBoundParameters.ContainsKey('Headless')) { $null = $Headless }

# If user asks for interactive mode but didn't explicitly set -Headless,
# default to showing the SDL window (otherwise it can look like QEMU didn't start).
if ($Interactive -and -not $PSBoundParameters.ContainsKey('Headless')) {
  $PSBoundParameters['Headless'] = $false
}

# Root convenience wrapper: allows running from repo root.
$script = [System.IO.Path]::Combine($PSScriptRoot, 'llm-baremetal', 'test-qemu.ps1')
if (-not (Test-Path $script)) {
  throw "llm-baremetal/test-qemu.ps1 not found at: $script"
}

& $script @PSBoundParameters
