param(
  [string]$OutputPath = "..\data\keurgui_state.json"
)

$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Resolve-Path (Join-Path $scriptDir "..")
$template = Join-Path $root "data\keurgui_state.template.json"
$out = Resolve-Path (Join-Path $scriptDir $OutputPath) -ErrorAction SilentlyContinue
if (-not $out) {
  $out = Join-Path $root "data\keurgui_state.json"
}

if (-not (Test-Path $template)) {
  throw "Template introuvable: $template"
}

New-Item -ItemType Directory -Path (Split-Path $out -Parent) -Force | Out-Null
Copy-Item $template $out -Force
Write-Host "Keurgui state seeded: $out" -ForegroundColor Green
