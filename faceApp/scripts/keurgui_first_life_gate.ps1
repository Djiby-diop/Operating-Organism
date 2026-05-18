param(
  [switch]$Force,
  [switch]$NoScreen
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$faceRoot = Resolve-Path (Join-Path $scriptDir "..")
$statePath = Join-Path $faceRoot "data\keurgui_state.json"
$bootstrap = Join-Path $scriptDir "start_keurgui_first_life.ps1"

if (-not (Test-Path $bootstrap)) {
  throw "Bootstrap script introuvable: $bootstrap"
}

$shouldBootstrap = $Force

if (-not (Test-Path $statePath)) {
  $shouldBootstrap = $true
} elseif (-not $Force) {
  try {
    $state = Get-Content $statePath -Raw | ConvertFrom-Json
    if (-not $state.first_life_completed) {
      $shouldBootstrap = $true
    }
  } catch {
    $shouldBootstrap = $true
  }
}

if ($shouldBootstrap) {
  Write-Host "[Keurgui] First-life bootstrap required." -ForegroundColor Cyan
  if ($NoScreen) {
    & $bootstrap -NoScreen
  } else {
    & $bootstrap
  }
} else {
  Write-Host "[Keurgui] First-life already completed, skipping bootstrap." -ForegroundColor DarkGray
}
