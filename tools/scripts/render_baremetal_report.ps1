Param(
    [string]$InputJson = "artifacts/baremetal_structure_report.json",
    [string]$OutputMarkdown = "artifacts/baremetal_structure_report.md"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (-not (Test-Path $InputJson)) {
    throw "Input JSON not found: $InputJson"
}

$data = Get-Content -Path $InputJson -Raw | ConvertFrom-Json
if (-not $data) {
    throw "Input JSON has no records: $InputJson"
}

$rows = @($data)
$total = $rows.Count
$compatOk = @($rows | Where-Object { $_.status -eq 'ok' }).Count
$strictOk = @($rows | Where-Object { $_.strict_status -eq 'ok' }).Count
$strictNeeds = @($rows | Where-Object { $_.strict_status -ne 'ok' }).Count

$lines = @()
$lines += "# Baremetal Structure Report"
$lines += ""
$lines += "- total modules: $total"
$lines += "- compat ok: $compatOk"
$lines += "- strict ok: $strictOk"
$lines += "- strict needs attention: $strictNeeds"
$lines += ""
$lines += "| module | compat | strict | README | include | src | header_only | core | bridge | Makefile |"
$lines += "|---|---|---|---|---|---|---|---|---|---|"

foreach ($r in ($rows | Sort-Object module)) {
    $lines += "| $($r.module) | $($r.status) | $($r.strict_status) | $($r.readme) | $($r.include) | $($r.src) | $($r.header_only) | $($r.core) | $($r.bridge) | $($r.makefile) |"
}

$dir = Split-Path -Parent $OutputMarkdown
if ($dir -and -not (Test-Path $dir)) {
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
}

$lines -join "`n" | Set-Content -Path $OutputMarkdown -NoNewline
Write-Host "Wrote markdown report: $OutputMarkdown"
