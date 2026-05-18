param(
  [string]$Model = '',
  [string]$ModelPath = '',
  [switch]$ListModels,
  [ValidateSet('auto','whpx','tcg','none')]
  [string]$Accel = 'tcg',
  [ValidateRange(512, 16384)]
  [int]$MemMB = 4096,
  [ValidateRange(30, 86400)]
  [int]$TimeoutSec = 900,
  [switch]$SkipBuild,
  [switch]$SkipPrebuild,
  [switch]$ForceAvx2
)

$ErrorActionPreference = 'Stop'

$workspaceRoot = $PSScriptRoot
$repoRoot = Join-Path $workspaceRoot 'llm-baremetal'
$diopRoot = Join-Path $repoRoot 'diop'
$registryPath = Join-Path $repoRoot '.diop_data\models\registry.json'
$defaultModelDir = Join-Path $repoRoot 'models'
$diopDataModelDir = Join-Path $repoRoot '.diop_data\models'
$stageDir = Join-Path $diopRoot 'runtime\qemu-models'
$harness = Join-Path $workspaceRoot 'test-qemu-autorun.ps1'

if (-not (Test-Path -LiteralPath $repoRoot)) {
  throw "Missing llm-baremetal repo at: $repoRoot"
}

if (-not (Test-Path -LiteralPath $harness)) {
  throw "Missing QEMU harness wrapper at: $harness"
}

function Get-NormalizedPath([string]$path) {
  return [System.IO.Path]::GetFullPath($path)
}

function Test-PathWithin([string]$candidate, [string]$root) {
  if (-not $candidate -or -not $root) { return $false }
  $candidatePath = Get-NormalizedPath $candidate
  $rootPath = (Get-NormalizedPath $root).TrimEnd('\\')
  $rootPrefix = "$rootPath\\"
  return $candidatePath.Equals($rootPath, [System.StringComparison]::OrdinalIgnoreCase) -or
    $candidatePath.StartsWith($rootPrefix, [System.StringComparison]::OrdinalIgnoreCase)
}

function Get-RegisteredModels {
  if (-not (Test-Path -LiteralPath $registryPath)) { return @() }

  $raw = Get-Content -LiteralPath $registryPath -Raw -ErrorAction SilentlyContinue
  if (-not $raw) { return @() }

  try {
    $items = $raw | ConvertFrom-Json
  } catch {
    return @()
  }

  $entries = @()
  foreach ($item in @($items)) {
    if (-not $item) { continue }
    $path = [string]$item.path
    if (-not $path) { continue }
    $exists = Test-Path -LiteralPath $path
    $entries += [PSCustomObject]@{
      Name = [string]$item.name
      Path = Get-NormalizedPath $path
      Format = [string]$item.format
      AddedAtUnix = [long]($item.added_at_unix | ForEach-Object { $_ })
      Exists = $exists
      Source = 'registered'
    }
  }

  return $entries
}

function Get-LocalModelFiles {
  $items = @()
  if (-not (Test-Path -LiteralPath $defaultModelDir)) { return $items }

  $files = Get-ChildItem -LiteralPath $defaultModelDir -File -ErrorAction SilentlyContinue |
    Where-Object {
      ($_.Extension -in '.gguf', '.bin') -and
      ($_.Name -notmatch '^tokenizer')
    } |
    Sort-Object LastWriteTimeUtc -Descending

  foreach ($file in $files) {
    $addedAtUnix = ([DateTimeOffset]$file.LastWriteTimeUtc).ToUnixTimeSeconds()
    $items += [PSCustomObject]@{
      Name = $file.BaseName
      Path = $file.FullName
      Format = $file.Extension.TrimStart('.')
      AddedAtUnix = $addedAtUnix
      Exists = $true
      Source = 'local-dir'
    }
  }

  return $items
}

function Get-PreferredLocalModel {
  $registered = Get-RegisteredModels | Where-Object { $_.Exists }
  $localFiles = Get-LocalModelFiles

  $preferredNames = @(
    'tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf',
    'stories15M.q8_0.gguf',
    'tinyllama-1.1b-chat-v1.0.Q8_0.gguf',
    'tinyllama-1.1b-chat-v1.0.Q2_K.gguf'
  )

  foreach ($preferred in $preferredNames) {
    $match = $registered | Where-Object { [System.IO.Path]::GetFileName($_.Path) -eq $preferred } | Select-Object -First 1
    if ($match) { return $match }

    $fileMatch = $localFiles | Where-Object { [System.IO.Path]::GetFileName($_.Path) -eq $preferred } | Select-Object -First 1
    if ($fileMatch) { return $fileMatch }
  }

  $recentRegistered = $registered | Sort-Object AddedAtUnix -Descending | Select-Object -First 1
  if ($recentRegistered) { return $recentRegistered }

  return $localFiles | Select-Object -First 1
}

function Resolve-DiopModel {
  $registered = Get-RegisteredModels
  $localFiles = Get-LocalModelFiles

  if ($ModelPath) {
    if (-not (Test-Path -LiteralPath $ModelPath)) {
      throw "ModelPath not found: $ModelPath"
    }

    $fullPath = Get-NormalizedPath $ModelPath
    $allowed = ($registered | Where-Object { $_.Exists -and $_.Path.Equals($fullPath, [System.StringComparison]::OrdinalIgnoreCase) } | Select-Object -First 1)
    if (-not $allowed) {
      $allowedRoots = @($defaultModelDir, $diopDataModelDir, $stageDir)
      $isAllowedRoot = $false
      foreach ($root in $allowedRoots) {
        if ((Test-Path -LiteralPath $root) -and (Test-PathWithin $fullPath $root)) {
          $isAllowedRoot = $true
          break
        }
      }
      if (-not $isAllowedRoot) {
        throw "ModelPath must be a DIOP-local model or a registered DIOP model: $fullPath"
      }
    }

    return [PSCustomObject]@{
      Name = [System.IO.Path]::GetFileNameWithoutExtension($fullPath)
      Path = $fullPath
      Format = [System.IO.Path]::GetExtension($fullPath).TrimStart('.')
      Source = 'explicit-path'
    }
  }

  if ($Model) {
    $registeredMatch = $registered |
      Where-Object {
        $_.Exists -and (
          $_.Name -eq $Model -or
          [System.IO.Path]::GetFileName($_.Path) -eq $Model -or
          [System.IO.Path]::GetFileNameWithoutExtension($_.Path) -eq $Model
        )
      } |
      Select-Object -First 1
    if ($registeredMatch) { return $registeredMatch }

    $localMatch = $localFiles |
      Where-Object {
        $_.Name -eq $Model -or
        [System.IO.Path]::GetFileName($_.Path) -eq $Model
      } |
      Select-Object -First 1
    if ($localMatch) { return $localMatch }

    throw "Unknown DIOP local model '$Model'. Use -ListModels to inspect available local models."
  }

  $preferred = Get-PreferredLocalModel
  if ($preferred) { return $preferred }

  throw 'No DIOP local model available. Register one with: python -m diop models add <name> <path>'
}

function Show-DiopModels {
  $registered = Get-RegisteredModels
  $localFiles = Get-LocalModelFiles

  Write-Host '[DIOP] Registered models' -ForegroundColor Cyan
  if (-not $registered) {
    Write-Host '  (none)' -ForegroundColor DarkGray
  } else {
    foreach ($item in $registered | Sort-Object AddedAtUnix -Descending) {
      $state = if ($item.Exists) { 'ok' } else { 'missing' }
      Write-Host ("  [{0}] {1} -> {2}" -f $state, $item.Name, $item.Path)
    }
  }

  Write-Host '[DIOP] Local model directory' -ForegroundColor Cyan
  Write-Host ("  {0}" -f $defaultModelDir)
  if (-not $localFiles) {
    Write-Host '  (no .gguf/.bin model found)' -ForegroundColor DarkGray
  } else {
    foreach ($item in $localFiles) {
      Write-Host ("  - {0}" -f [System.IO.Path]::GetFileName($item.Path))
    }
  }
}

if ($ListModels) {
  Show-DiopModels
  exit 0
}

$resolved = Resolve-DiopModel

New-Item -ItemType Directory -Path $stageDir -Force | Out-Null
$stagedModelPath = Join-Path $stageDir ([System.IO.Path]::GetFileName($resolved.Path))
Copy-Item -LiteralPath $resolved.Path -Destination $stagedModelPath -Force

$repoRelativeModel = $stagedModelPath.Substring((Get-NormalizedPath $repoRoot).TrimEnd('\\').Length).TrimStart('\\') -replace '\\', '/'

Write-Host '[DIOP] QEMU local-model smoke test' -ForegroundColor Cyan
Write-Host ("  Model:   {0}" -f $resolved.Path) -ForegroundColor Gray
Write-Host ("  Staged:  {0}" -f $repoRelativeModel) -ForegroundColor Gray
Write-Host ("  Source:  {0}" -f $resolved.Source) -ForegroundColor Gray
Write-Host ("  Accel:   {0}" -f $Accel) -ForegroundColor Gray
Write-Host ("  Memory:  {0} MB" -f $MemMB) -ForegroundColor Gray

$callParams = @{
  Mode = 'oo_consult_smoke'
  Accel = $Accel
  MemMB = $MemMB
  TimeoutSec = $TimeoutSec
  ModelBin = $repoRelativeModel
}

if ($SkipBuild) { $callParams['SkipBuild'] = $true }
if ($SkipPrebuild) { $callParams['SkipPrebuild'] = $true }
if ($ForceAvx2) { $callParams['ForceAvx2'] = $true }

& $harness @callParams
exit $LASTEXITCODE