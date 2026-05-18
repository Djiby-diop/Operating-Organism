param(
    [string]$ImagePath,
    [string]$OvmfPath,
    [ValidateSet('whpx','tcg','none')][string]$Accel = 'tcg',
    [int]$MemMB = 1024,
    [int]$TimeoutSec = 120
)

$ErrorActionPreference = 'Stop'

# Root convenience wrapper: allows running from repo root.
$script = [System.IO.Path]::Combine($PSScriptRoot, 'llm-baremetal', 'test-qemu-manual-smoke.ps1')
if (-not (Test-Path $script)) {
    throw "llm-baremetal/test-qemu-manual-smoke.ps1 not found at: $script"
}

& $script @PSBoundParameters
