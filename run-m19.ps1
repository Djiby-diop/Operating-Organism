[CmdletBinding(PositionalBinding = $false)]
param(
  # If set, runs in static mode (no QEMU). Useful for quick compare/regenerate.
  [switch]$Static,

  # If set, updates the M19 baseline from current results before compare.
  [switch]$UpdateBaseline,

  # If set, also extracts metrics and produces CI report (runtime only).
  [switch]$WithMetrics,

  [ValidateSet('auto','whpx','tcg','none')]
  [string]$Accel = 'tcg',

  [ValidateRange(512, 8192)]
  [int]$MemMB = 4096,

  # Optional: overrides reliability timeout (seconds). 0 means "let reliability decide".
  [ValidateRange(0, 7200)]
  [int]$TimeoutSec = 0,

  # Any additional args are forwarded as-is to llm-baremetal/reliability.ps1
  # (e.g. -SkipPreflight -SkipBuild -SkipPrebuild -M16ExtractMetrics ...).
  [Parameter(ValueFromRemainingArguments = $true)]
  [string[]]$ForwardArgs
)

$ErrorActionPreference = 'Stop'

$repoRoot = $PSScriptRoot
$reliability = Join-Path $repoRoot 'llm-baremetal' 'reliability.ps1'
if (-not (Test-Path -LiteralPath $reliability)) {
  throw "Missing: $reliability"
}

$callArgs = @()
if (-not $Static) { $callArgs += '-RunQemu' }
$callArgs += '-M19EnableBenchmarkPack'
$callArgs += @('-Accel', $Accel)
$callArgs += @('-MemMB', "$MemMB")

if ($TimeoutSec -gt 0) {
  $callArgs += @('-TimeoutSec', "$TimeoutSec")
}

if ($UpdateBaseline) {
  $callArgs += '-M19UpdateBaseline'
}

if ($WithMetrics) {
  if ($Static) {
    throw "-WithMetrics requires runtime mode (omit -Static)."
  }
  $callArgs += @('-M16ExtractMetrics', '-M17EnableCIReport')
}

# Forward any additional args directly to reliability.ps1 (e.g. -SkipBuild, -SkipPreflight, ...)
if ($ForwardArgs -and $ForwardArgs.Count -gt 0) {
  $callArgs += $ForwardArgs
}

& pwsh -NoProfile -ExecutionPolicy Bypass -File $reliability @callArgs
exit $LASTEXITCODE
