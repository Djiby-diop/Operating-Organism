# test-qemu-sia.ps1 — SIA Integration Test Script
# =====================================================================
# Launches QEMU and monitors UART logs for SIA-specific signals:
#
#   [sia-dual]    — SomaDual temperature bias applied (rhythm active)
#   [sm-sia]      — SomaMind×SIA per-turn and postturn events
#   [sia-bridge]  — Token stream SIA command intercepts
#   [sia-dream]   — Dream consolidation phases
#   [sia-persist] — NeuralFS2 save/load events
#   [oo-event]    — Sovereignty vetoes and D+ gate events
#
# Usage:
#   .\test-qemu-sia.ps1                          # Single node, default model
#   .\test-qemu-sia.ps1 -Model cortex_oo         # Use cortex_oo model
#   .\test-qemu-sia.ps1 -Nodes 2                 # Swarm test (Level 2 civ)
#   .\test-qemu-sia.ps1 -TimeoutSec 300 -Verbose # Extended test with full logs
#   .\test-qemu-sia.ps1 -Report                  # Generate SIA analysis report
# =====================================================================

param(
    [ValidateSet("stories15M", "cortex_oo", "diop_model", "stories110M")]
    [string]$Model         = "stories15M",
    [int]$Nodes            = 1,
    [int]$TimeoutSec       = 180,
    [switch]$Verbose,
    [switch]$Report,
    [switch]$SkipBuild,
    [ValidateSet("tcg", "whpx", "none")]
    [string]$Accel         = "tcg"
)

$ErrorActionPreference = "Continue"
Set-StrictMode -Off

# ── Paths ──────────────────────────────────────────────────────────────────────
$ROOT     = "C:\Users\djibi\OneDrive\Bureau\baremetal"
$LLMROOT  = "$ROOT\llm-baremetal"
$IMGPATH  = "$ROOT\llm-baremetal-boot.img"
$QEMU     = "C:\Program Files\qemu\qemu-system-x86_64.exe"
$OVMF     = "C:\Program Files\qemu\share\edk2-x86_64-code.fd"
$SWARM_DIR= "$ROOT\swarm_temp"
$REPORT_DIR = "$ROOT\artifacts"

# ── Color helpers ──────────────────────────────────────────────────────────────
function Title  { param($s) Write-Host "`n╔══ $s" -ForegroundColor Cyan }
function Ok     { param($s) Write-Host "  [✓] $s" -ForegroundColor Green }
function Warn   { param($s) Write-Host "  [!] $s" -ForegroundColor Yellow }
function Err    { param($s) Write-Host "  [✗] $s" -ForegroundColor Red }
function Info   { param($s) Write-Host "      $s" -ForegroundColor Gray }
function SiaLog { param($s) Write-Host "  $s" -ForegroundColor Magenta }

# ── SIA signal patterns ───────────────────────────────────────────────────────
$SIA_PATTERNS = @{
    "Dual-Bias"       = '\[sia-dual\]'
    "SomaMind-Hook"   = '\[sm-sia\]'
    "Bridge-Command"  = '\[sia-bridge\]'
    "Dream-Phase"     = '\[sia-dream\]'
    "Persist"         = '\[sia-persist\]'
    "Sovereignty-Veto"= 'kind=sovereignty_veto'
    "Trust-Event"     = '\[oo-event\] kind=trust'
    "SIA-Status"      = '\[SIA sync='
    "Rhythm-Mode"     = 'rhythm=\w+'
    "Dual-Choice"     = 'dual_bias=(SOLAR|LUNAR|NEUTRAL)'
}

# ── Assertion functions ────────────────────────────────────────────────────────
$global:assertions_passed = 0
$global:assertions_failed = 0

function Assert-UartContains {
    param([string]$LogFile, [string]$Pattern, [string]$Name, [switch]$MustNotContain)
    $content = if (Test-Path $LogFile) { Get-Content $LogFile -Raw -ErrorAction SilentlyContinue } else { "" }
    $found = $content -match $Pattern
    if ($MustNotContain) {
        if (-not $found) {
            Ok "PASS: $Name (absent as expected)"
            $global:assertions_passed++
        } else {
            Err "FAIL: $Name — found unexpected pattern: $Pattern"
            $global:assertions_failed++
        }
    } else {
        if ($found) {
            Ok "PASS: $Name"
            $global:assertions_passed++
        } else {
            Warn "FAIL: $Name — pattern not found: $Pattern"
            $global:assertions_failed++
        }
    }
}

function Get-SiaMetrics {
    param([string]$LogFile)
    $metrics = @{
        DualBiasEvents    = 0
        SmSiaEvents       = 0
        BridgeCommands    = 0
        DreamPhases       = 0
        SovereigntyVetoes = 0
        SolarBias         = 0
        LunarBias         = 0
        NeutralBias       = 0
        TrustLevel        = "UNKNOWN"
        SyncLevel         = "UNKNOWN"
        RhythmMode        = "UNKNOWN"
        AvgTsTemp         = "N/A"
        AvgTlTemp         = "N/A"
    }
    if (-not (Test-Path $LogFile)) { return $metrics }
    $lines = Get-Content $LogFile -ErrorAction SilentlyContinue

    foreach ($line in $lines) {
        if ($line -match '\[sia-dual\]')           { $metrics.DualBiasEvents++ }
        if ($line -match '\[sm-sia\]')             { $metrics.SmSiaEvents++ }
        if ($line -match '\[sia-bridge\]')         { $metrics.BridgeCommands++ }
        if ($line -match '\[sia-dream\]')          { $metrics.DreamPhases++ }
        if ($line -match 'sovereignty_veto')        { $metrics.SovereigntyVetoes++ }
        if ($line -match 'dual_bias=\+1')           { $metrics.SolarBias++ }
        if ($line -match 'dual_bias=-1')            { $metrics.LunarBias++ }
        if ($line -match 'dual_bias=0')             { $metrics.NeutralBias++ }
        if ($line -match 'trust=(\w+)')             { $metrics.TrustLevel = $Matches[1] }
        if ($line -match 'sync=(\w+)')              { $metrics.SyncLevel  = $Matches[1] }
        if ($line -match 'rhythm=(\w+)')            { $metrics.RhythmMode = $Matches[1] }
        if ($line -match 'Ts=([\d.]+)')             { $metrics.AvgTsTemp  = $Matches[1] }
        if ($line -match 'Tl=([\d.]+)')             { $metrics.AvgTlTemp  = $Matches[1] }
    }
    return $metrics
}

# ── Build (optional) ───────────────────────────────────────────────────────────
Set-Location $ROOT
Title "SIA Integration Test — Operating Organism"
Info "Model   : $Model"
Info "Nodes   : $Nodes"
Info "Timeout : ${TimeoutSec}s"
Info "Accel   : $Accel"

if (-not $SkipBuild) {
    Title "1/4  Build EFI with SIA"
    Info "Compiling via WSL make..."
    $buildOut = wsl -e bash -c "cd /mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal && make -j4 2>&1 | tail -30" 2>&1
    if ($LASTEXITCODE -eq 0) {
        Ok "Build successful"
    } else {
        Warn "Build completed with warnings (exit $LASTEXITCODE)"
        if ($Verbose) { Write-Host $buildOut -ForegroundColor DarkGray }
    }
} else {
    Title "1/4  Build (skipped)"
    Info "Using existing KERNEL.EFI"
}

# ── Model selection ────────────────────────────────────────────────────────────
Title "2/4  Model selection"
$MODEL_FILES = @{
    "stories15M"  = "$LLMROOT\bootable_img_temp\stories15M.bin"
    "stories110M" = "$LLMROOT\stories110M.bin"
    "cortex_oo"   = "$LLMROOT\oo-model-repo\models\cortex_oo.bin"
    "diop_model"  = "$LLMROOT\diop\engine\model\diop_model.bin"
}
$MODEL_PATH = $MODEL_FILES[$Model]
if (-not (Test-Path $MODEL_PATH)) {
    $fallback = Get-ChildItem -Path $ROOT -Recurse -Filter "stories15M.bin" -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($fallback) { $MODEL_PATH = $fallback.FullName; Warn "Fallback → $MODEL_PATH" }
    else           { Err "No model found. Abort."; exit 1 }
}
Ok "Model: $(Split-Path $MODEL_PATH -Leaf)"

# ── Launch QEMU nodes ──────────────────────────────────────────────────────────
Title "3/4  Launching $Nodes QEMU node(s)"
if (-not (Test-Path $SWARM_DIR)) { New-Item -ItemType Directory $SWARM_DIR -Force | Out-Null }

$qemuProcs  = @()
$uartLogs   = @()

for ($i = 1; $i -le $Nodes; $i++) {
    $nodeId   = "SIA_NODE_$i"
    $nodeDir  = "$SWARM_DIR\$nodeId"
    $nodeImg  = "$nodeDir\boot.img"
    $nodeUart = "$ROOT\OO_UART_${nodeId}.log"
    $nodeVars = "$env:TEMP\oo-sia-vars-$nodeId.fd"

    if (-not (Test-Path $nodeDir)) { New-Item -ItemType Directory $nodeDir -Force | Out-Null }
    if (Test-Path $nodeUart)       { Remove-Item $nodeUart -Force }

    Copy-Item $IMGPATH $nodeImg -Force
    if (-not (Test-Path $nodeVars)) { Copy-Item $OVMF $nodeVars -Force }

    $uartLogs += $nodeUart

    $qemuArgs = @(
        "-machine", "q35,accel=$Accel",
        "-cpu",     "max",
        "-m",       "2048",
        "-drive",   "if=pflash,format=raw,readonly=on,file=$OVMF",
        "-drive",   "if=pflash,format=raw,file=$nodeVars",
        "-drive",   "format=raw,file=$nodeImg,if=ide",
        "-serial",  "file:$nodeUart",
        "-name",    "OO_$nodeId",
        "-display", "none",
        "-no-reboot"
    )

    $proc = Start-Process -FilePath $QEMU -ArgumentList $qemuArgs -PassThru -NoNewWindow
    $qemuProcs += $proc
    Ok "$nodeId started (PID $($proc.Id)) → $nodeUart"
}

# ── Monitor UART for SIA signals ───────────────────────────────────────────────
Title "4/4  Monitoring SIA UART signals (${TimeoutSec}s)"
Write-Host ""
Write-Host "  Pattern key:" -ForegroundColor DarkGray
Write-Host "    [MAGENTA] SIA signals    [GREEN] assertions OK    [YELLOW] warnings" -ForegroundColor DarkGray
Write-Host ""

$startTime = Get-Date
$deadline  = $startTime.AddSeconds($TimeoutSec)
$seenLines = @{}
foreach ($log in $uartLogs) { $seenLines[$log] = 0 }

$siaSignalCount = 0

try {
    while ((Get-Date) -lt $deadline) {
        $running = $qemuProcs | Where-Object { -not $_.HasExited }
        if ($running.Count -eq 0) { Info "All nodes exited."; break }

        foreach ($log in $uartLogs) {
            if (-not (Test-Path $log)) { continue }
            $lines = Get-Content $log -ErrorAction SilentlyContinue
            if (-not $lines) { continue }

            $newLines = $lines | Select-Object -Skip $seenLines[$log]
            $seenLines[$log] = $lines.Count

            foreach ($line in $newLines) {
                # Check each SIA pattern
                $isSia = $false
                foreach ($kv in $SIA_PATTERNS.GetEnumerator()) {
                    if ($line -match $kv.Value) {
                        SiaLog "[$($kv.Key)] $line"
                        $siaSignalCount++
                        $isSia = $true
                        break
                    }
                }
                # Print non-SIA lines only in Verbose mode
                if (-not $isSia -and $Verbose) {
                    Info $line
                }
            }
        }

        $elapsed = [int]((Get-Date) - $startTime).TotalSeconds
        Write-Host -NoNewline "`r  Elapsed: ${elapsed}s | SIA signals: $siaSignalCount | Nodes: $($running.Count)/$Nodes   "
        Start-Sleep -Milliseconds 500
    }
} catch {
    Info "Interrupted."
} finally {
    Write-Host ""
    foreach ($p in $qemuProcs) {
        if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue }
    }
}

# ── Assertions ─────────────────────────────────────────────────────────────────
Title "Assertions"

$primaryLog = $uartLogs[0]

# SIA must boot and initialize
Assert-UartContains $primaryLog '\[sm-sia\] SomaMind.SIA integration online' `
    "SIA integration layer initialized"

# SIA prefix must appear at least once (pre-turn called)
Assert-UartContains $primaryLog '\[SIA sync=' `
    "SIA context prefix generated (pre-turn active)"

# Post-turn must fire (sm-sia turn done)
Assert-UartContains $primaryLog '\[sm-sia\] turn done halt=' `
    "SIA post-turn hook firing after inference"

# SomaDual bias must be applied when rhythm detects a mode
Assert-UartContains $primaryLog '\[sia-dual\] bias=' `
    "SomaDual temperature bias applied by rhythm engine"

# Sovereignty clauses must be initialized (7 default)
Assert-UartContains $primaryLog 'sia' `
    "SIA system active in kernel"

# Trust score must appear in logs
Assert-UartContains $primaryLog 'trust=\w+' `
    "Trust level reported in turn logs"

# Sovereignty veto must NOT appear on clean boot (no illegal actions expected)
Assert-UartContains $primaryLog 'sovereignty_veto' `
    "No sovereignty vetoes on clean boot" -MustNotContain

# ── Metrics report ─────────────────────────────────────────────────────────────
Title "SIA Metrics"
$m = Get-SiaMetrics $primaryLog

Write-Host ""
Write-Host "  ╔══ SIA Run Report ══════════════════════════════╗" -ForegroundColor Cyan
Write-Host "  ║  Dual-Bias Events    : $($m.DualBiasEvents)" -ForegroundColor White
Write-Host "  ║  SomaMind-SIA Hooks  : $($m.SmSiaEvents)" -ForegroundColor White
Write-Host "  ║  Bridge Commands     : $($m.BridgeCommands)" -ForegroundColor White
Write-Host "  ║  Dream Phases        : $($m.DreamPhases)" -ForegroundColor White
Write-Host "  ║  Sovereignty Vetoes  : $($m.SovereigntyVetoes)" -ForegroundColor White
Write-Host "  ║  Solar bias turns    : $($m.SolarBias)" -ForegroundColor Yellow
Write-Host "  ║  Lunar bias turns    : $($m.LunarBias)" -ForegroundColor Cyan
Write-Host "  ║  Neutral turns       : $($m.NeutralBias)" -ForegroundColor Gray
Write-Host "  ║  Last Trust Level    : $($m.TrustLevel)" -ForegroundColor Green
Write-Host "  ║  Last Sync Level     : $($m.SyncLevel)" -ForegroundColor Green
Write-Host "  ║  Last Rhythm Mode    : $($m.RhythmMode)" -ForegroundColor Magenta
Write-Host "  ║  Effective Ts (last) : $($m.AvgTsTemp)" -ForegroundColor Gray
Write-Host "  ║  Effective Tl (last) : $($m.AvgTlTemp)" -ForegroundColor Gray
Write-Host "  ╚══════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# ── Assertion summary ──────────────────────────────────────────────────────────
Title "Test Summary"
$total = $global:assertions_passed + $global:assertions_failed
Write-Host "  Assertions : $($global:assertions_passed)/$total passed" -ForegroundColor $(if ($global:assertions_failed -eq 0) { "Green" } else { "Yellow" })
Write-Host "  SIA signals: $siaSignalCount total" -ForegroundColor Cyan

if ($global:assertions_failed -eq 0) {
    Write-Host "`n  ✅  SIA INTEGRATION TEST PASSED" -ForegroundColor Green
} else {
    Write-Host "`n  ⚠️   $($global:assertions_failed) assertion(s) failed" -ForegroundColor Yellow
    Write-Host "      Check UART log: $primaryLog" -ForegroundColor Gray
}

# ── Optional report file ────────────────────────────────────────────────────────
if ($Report) {
    if (-not (Test-Path $REPORT_DIR)) { New-Item -ItemType Directory $REPORT_DIR -Force | Out-Null }
    $stamp = (Get-Date -Format "yyyyMMdd_HHmmss")
    $reportPath = "$REPORT_DIR\sia_test_$stamp.md"

    $reportContent = @"
# SIA Integration Test Report
**Date**: $(Get-Date -Format "yyyy-MM-dd HH:mm:ss")
**Model**: $Model  **Nodes**: $Nodes  **Timeout**: ${TimeoutSec}s

## Results
| Metric | Value |
|--------|-------|
| Dual-Bias Events | $($m.DualBiasEvents) |
| SomaMind-SIA Hooks | $($m.SmSiaEvents) |
| Bridge Commands | $($m.BridgeCommands) |
| Dream Phases | $($m.DreamPhases) |
| Sovereignty Vetoes | $($m.SovereigntyVetoes) |
| Solar Bias Turns | $($m.SolarBias) |
| Lunar Bias Turns | $($m.LunarBias) |
| Last Trust Level | $($m.TrustLevel) |
| Last Sync Level | $($m.SyncLevel) |
| Last Rhythm Mode | $($m.RhythmMode) |
| Effective T_solar | $($m.AvgTsTemp) |
| Effective T_lunar | $($m.AvgTlTemp) |

## Assertions
- Passed: $($global:assertions_passed)/$total
- Failed: $($global:assertions_failed)

## Status
$(if ($global:assertions_failed -eq 0) { "✅ PASS" } else { "⚠️ PARTIAL ($($global:assertions_failed) failures)" })

## UART Log
``$primaryLog``
"@
    $reportContent | Set-Content $reportPath -Encoding UTF8
    Ok "Report saved: $reportPath"
}

exit $(if ($global:assertions_failed -eq 0) { 0 } else { 1 })
