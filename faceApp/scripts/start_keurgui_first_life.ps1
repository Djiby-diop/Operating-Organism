param(
  [switch]$NoScreen,
  [string]$StatePath = "..\data\keurgui_state.json"
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$faceRoot = Resolve-Path (Join-Path $scriptDir "..")
$root = Resolve-Path (Join-Path $faceRoot "..")
$desktop = Join-Path $root "desktop_display"
$screenScript = Join-Path $desktop "keurgui_screen2.py"
$stateFull = Resolve-Path (Join-Path $scriptDir $StatePath) -ErrorAction SilentlyContinue
if (-not $stateFull) {
  $stateFull = Join-Path $faceRoot "data\keurgui_state.json"
}

if (-not (Test-Path $stateFull)) {
  & (Join-Path $scriptDir "seed_keurgui_defaults.ps1") -OutputPath $StatePath
}

$data = Get-Content $stateFull -Raw | ConvertFrom-Json
if (-not $data.created_at) {
  $data.created_at = (Get-Date).ToUniversalTime().ToString("o")
}

if (-not $data.PSObject.Properties.Name.Contains("quickstart")) {
  $data | Add-Member -NotePropertyName quickstart -NotePropertyValue @{
    enabled = $true
    step = 1
    steps_total = 4
    completed = $false
    last_action = "bootstrap_initialized"
  }
}

if (-not $data.first_life_completed) {
  $data.quickstart.enabled = $true
  if (-not $data.quickstart.step) { $data.quickstart.step = 1 }
  if (-not $data.quickstart.steps_total) { $data.quickstart.steps_total = 4 }
  if (-not $data.quickstart.last_action) { $data.quickstart.last_action = "bootstrap_initialized" }
}

$data | ConvertTo-Json -Depth 8 | Set-Content $stateFull -Encoding UTF8

Write-Host "Keurgui first-life bootstrap done." -ForegroundColor Green
Write-Host "State: $stateFull"

if (-not $NoScreen) {
  if (-not (Test-Path $screenScript)) {
    Write-Warning "Screen script introuvable: $screenScript"
    exit 0
  }

  $python = Get-Command python -ErrorAction SilentlyContinue
  if (-not $python) {
    $python = Get-Command python3 -ErrorAction SilentlyContinue
  }
  if (-not $python) {
    Write-Warning "Python introuvable, ecran non lance."
    exit 0
  }

  $env:KEURGUI_STATE_PATH = $stateFull
  Start-Process -FilePath $python.Source -ArgumentList "`"$screenScript`"" -WorkingDirectory $desktop
  Write-Host "Keurgui screen launched." -ForegroundColor Cyan
}
