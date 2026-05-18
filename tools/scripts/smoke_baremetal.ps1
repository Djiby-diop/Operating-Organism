Param(
    [switch]$FailOnMissing,
    [switch]$FailOnStrictMissing,
    [switch]$Json
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
Set-Location $repoRoot

$targets = Get-ChildItem -Directory | Where-Object { $_.Name -like '*-baremetal' } | Sort-Object Name
if (-not $targets -or $targets.Count -eq 0) {
    throw "No *-baremetal modules found in $repoRoot"
}

$results = @()
foreach ($t in $targets) {
    $modulePath = $t.FullName
    $readme = Test-Path (Join-Path $modulePath 'README.md')
    $include = Test-Path (Join-Path $modulePath 'include')
    $src = Test-Path (Join-Path $modulePath 'src')
    $core = Test-Path (Join-Path $modulePath 'core')
    $bridge = Test-Path (Join-Path $modulePath 'bridge')
    $makefile = Test-Path (Join-Path $modulePath 'Makefile')

    $strictScore = 0
    if ($readme) { $strictScore += 1 }
    if ($include) { $strictScore += 1 }
    if ($src) { $strictScore += 1 }

    $hasInterfaceLayer = $include -or $core -or $bridge
    $headerOnly = $include -and -not $src -and -not $core
    $hasSourceLayer = $src -or $core -or $headerOnly

    $compatScore = 0
    if ($readme) { $compatScore += 1 }
    if ($hasInterfaceLayer) { $compatScore += 1 }
    if ($hasSourceLayer) { $compatScore += 1 }

    $strictStatus = if ($strictScore -eq 3) { 'ok' } else { 'needs-attention' }
    $status = if ($compatScore -eq 3) { 'ok' } else { 'needs-attention' }

    $results += [pscustomobject]@{
        module = $t.Name
        status = $status
        strict_status = $strictStatus
        readme = $readme
        include = $include
        src = $src
        header_only = $headerOnly
        core = $core
        bridge = $bridge
        makefile = $makefile
        strict_score = $strictScore
        compat_score = $compatScore
    }
}

$missingCount = @($results | Where-Object { $_.status -ne 'ok' }).Count
$strictMissingCount = @($results | Where-Object { $_.strict_status -ne 'ok' }).Count

if ($Json) {
    $results | ConvertTo-Json -Depth 4
} else {
    Write-Host "baremetal smoke report (root=$repoRoot)"
    foreach ($r in $results) {
        Write-Host ("[{0}] {1} | strict={2} README={3} include={4} src={5} core={6} bridge={7} Makefile={8}" -f $r.status, $r.module, $r.strict_status, $r.readme, $r.include, $r.src, $r.core, $r.bridge, $r.makefile)
    }
    Write-Host ("summary: modules={0} compat_ok={1} compat_needs_attention={2} strict_ok={3} strict_needs_attention={4}" -f $results.Count, ($results.Count - $missingCount), $missingCount, ($results.Count - $strictMissingCount), $strictMissingCount)
}

if ($FailOnMissing -and $missingCount -gt 0) {
    Write-Error ("baremetal smoke failed: {0} module(s) need attention" -f $missingCount)
    exit 1
}

if ($FailOnStrictMissing -and $strictMissingCount -gt 0) {
    Write-Error ("baremetal strict smoke failed: {0} module(s) need strict structure harmonization" -f $strictMissingCount)
    exit 1
}
