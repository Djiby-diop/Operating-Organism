<#
.SYNOPSIS
    oo-run.ps1 — Operating Organism boot launcher

.DESCRIPTION
    1. Pre-flight: verify organs + homeostasis invariants
    2. Boot: launch QEMU (UEFI → llama2.efi → united-bus → cortex → REPL)
    3. Post-boot: parse OO_UART.log, show organism health report

.PARAMETER Interactive
    Show QEMU SDL window (default: headless, UART to terminal)

.PARAMETER TailUart
    Stream UART output live while booting

.PARAMETER MemMB
    Guest RAM in MB (default: 4096)

.PARAMETER Accel
    QEMU accelerator: tcg (safe/slow) | whpx (fast, needs Hyper-V) | auto

.PARAMETER SkipPreflight
    Skip organ pre-flight checks (boot immediately)

.PARAMETER TimeoutSec
    Auto-shutdown after N seconds (0 = no timeout, default: 0)

.EXAMPLE
    pwsh oo-run.ps1
    pwsh oo-run.ps1 -Interactive
    pwsh oo-run.ps1 -TailUart -TimeoutSec 30
    pwsh oo-run.ps1 -Accel whpx -MemMB 8192
#>

param(
    [switch]$Interactive,
    [switch]$TailUart,
    [ValidateSet('tcg','whpx','auto')]
    [string]$Accel = 'tcg',
    [ValidateRange(512, 16384)]
    [int]$MemMB = 4096,
    [switch]$SkipPreflight,
    [int]$TimeoutSec = 0
)

$ErrorActionPreference = "Stop"
$ROOT = $PSScriptRoot
if (-not $ROOT) { $ROOT = "C:\Users\djibi\OneDrive\Bureau\baremetal" }

$QEMU      = "C:\Program Files\qemu\qemu-system-x86_64.exe"
$OVMF_CODE = "C:\Program Files\qemu\share\edk2-x86_64-code.fd"
$OVMF_VARS_SRC = "C:\Program Files\qemu\share\edk2-i386-vars.fd"
$OVMF_VARS_TMP = "$env:TEMP\oo-edk2-vars.fd"
$IMG       = "$ROOT\llm-baremetal\llm-baremetal-boot.img"
$OO_UART   = "$ROOT\llm-baremetal\OO_UART.log"
$SERIAL_LOG = "$env:TEMP\oo-serial.txt"

# ── Colour helpers ────────────────────────────────────────────────────────────
function OK($msg)   { Write-Host "  [OK]  $msg" -ForegroundColor Green }
function WARN($msg) { Write-Host "  [!!]  $msg" -ForegroundColor Yellow }
function ERR($msg)  { Write-Host "  [XX]  $msg" -ForegroundColor Red }
function HDR($msg)  { Write-Host "`n$msg" -ForegroundColor Magenta }

# ── Banner ────────────────────────────────────────────────────────────────────
Write-Host ""
Write-Host "  ██████╗  ██████╗      ██████╗  ██████╗ " -ForegroundColor Cyan
Write-Host "  ██╔═══██╗██╔═══██╗    ██╔══██╗██╔═══██╗" -ForegroundColor Cyan
Write-Host "  ██║   ██║██║   ██║    ██████╔╝██║   ██║" -ForegroundColor Cyan
Write-Host "  ██║   ██║██║   ██║    ██╔══██╗██║   ██║" -ForegroundColor Cyan
Write-Host "  ╚██████╔╝╚██████╔╝    ██████╔╝╚██████╔╝" -ForegroundColor Cyan
Write-Host "   ╚═════╝  ╚═════╝     ╚═════╝  ╚═════╝ " -ForegroundColor Cyan
Write-Host ""
Write-Host "  Operating Organism — Boot Launcher" -ForegroundColor White
Write-Host "  $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')" -ForegroundColor Gray
Write-Host ""

# ── PRE-FLIGHT ────────────────────────────────────────────────────────────────
if (-not $SkipPreflight) {
    HDR "── Pre-Flight: Organ Inventory ──────────────────────────────"

    $organs = @(
        @{ Name="united-baremetal";        Bio="Cardiovascular (IPC bus)" },
        @{ Name="sense-baremetal";         Bio="Sensory Organs" },
        @{ Name="reflex-baremetal";        Bio="Brainstem / Spinal Cord" },
        @{ Name="kernel-baremetal";        Bio="Musculoskeletal (scheduler)" },
        @{ Name="memory-baremetal";        Bio="Memory System" },
        @{ Name="vital-baremetal";         Bio="Endocrine (homeostasis FSM)" },
        @{ Name="identity-baremetal";      Bio="Identity / DNA" },
        @{ Name="network-baremetal";       Bio="Respiratory System" },
        @{ Name="proprioception-baremetal";Bio="Proprioception" },
        @{ Name="regen-baremetal";         Bio="Recovery / Mutation" },
        @{ Name="vocal-baremetal";         Bio="Speech / UART" },
        @{ Name="shadow-baremetal";        Bio="Immune / Boundary" },
        @{ Name="swarm-baremetal";         Bio="Lymphatic / Collective" },
        @{ Name="evolution-baremetal";     Bio="Reproductive / LoRA" },
        @{ Name="dream-baremetal";         Bio="Sleep / Consolidation" },
        @{ Name="bot-baremetal";           Bio="Immune (instinct+agents)" }
    )

    $organ_ok = 0; $organ_warn = 0
    foreach ($o in $organs) {
        $path = "$ROOT\$($o.Name)"
        $has_src = (Test-Path "$path\src") -and ((Get-ChildItem "$path\src\*.c" -ErrorAction SilentlyContinue).Count -gt 0)
        $has_inc = Test-Path "$path\include"
        if ($has_src -and $has_inc) {
            $files = (Get-ChildItem "$path\src\*.c").Count
            OK  "$($o.Name.PadRight(30)) $($o.Bio) [$files src]"
            $organ_ok++
        } else {
            WARN "$($o.Name.PadRight(30)) $($o.Bio) [incomplete]"
            $organ_warn++
        }
    }

    Write-Host ""
    Write-Host "  Organs: $organ_ok OK, $organ_warn warnings" -ForegroundColor $(if ($organ_warn -eq 0) { "Green" } else { "Yellow" })

    HDR "── Pre-Flight: Homeostasis Invariants ───────────────────────"

    $inv_pass = 0; $inv_fail = 0

    function Inv($n, $label, $cond, $detail="") {
        if ($cond) { OK  "INV-$n: $label"; $script:inv_pass++ }
        else        { ERR "INV-$n: $label $detail"; $script:inv_fail++ }
    }

    Inv 1 "QEMU available"           (Test-Path $QEMU)
    Inv 2 "OVMF firmware available"  (Test-Path $OVMF_CODE)
    Inv 3 "Boot image present"       (Test-Path $IMG) "(run 'make' in llm-baremetal/ from WSL if missing)"
    Inv 4 "KERNEL.EFI present"       ((Test-Path "$ROOT\KERNEL.EFI") -or (Test-Path "$ROOT\llm-baremetal\llama2.efi"))
    Inv 5 "United-bus ring buffer"   (Test-Path "$ROOT\united-baremetal\src\united_bus.c")
    Inv 6 "Reflex engine armed"      (Test-Path "$ROOT\reflex-baremetal\include\nervous_system.h")
    Inv 7 "D+ policy gate"           (Test-Path "$ROOT\llm-baremetal\thalamic-bloom\oo_thalamic_bridge.h")
    Inv 8 "Memory allocator"         (Test-Path "$ROOT\memory-baremetal\src\bio_alloc.c")
    Inv 9 "Cortex EFI entry"         (Test-Path "$ROOT\llm-baremetal\llama2.efi")
    Inv 10 "Colony config"           (Test-Path "$ROOT\oo-host\colony_url.txt") 2>$null

    Write-Host ""
    Write-Host "  Invariants: $inv_pass/10 PASS" -ForegroundColor $(if ($inv_fail -eq 0) { "Green" } else { "Yellow" })

    if (-not (Test-Path $QEMU)) {
        Write-Host "`n  [FATAL] QEMU not found. Install from https://www.qemu.org/download/" -ForegroundColor Red
        exit 1
    }
    if (-not (Test-Path $IMG)) {
        Write-Host "`n  [FATAL] Boot image missing: $IMG" -ForegroundColor Red
        Write-Host "  From WSL: cd llm-baremetal && make" -ForegroundColor Yellow
        exit 1
    }
}

# ── BOOT ──────────────────────────────────────────────────────────────────────
HDR "── Boot: UEFI → llama2.efi → United-Bus → Cortex → REPL ────"
Write-Host "  Accel: $Accel  |  RAM: ${MemMB}MB  |  Image: $(Split-Path $IMG -Leaf)" -ForegroundColor Gray
Write-Host ""

# Fresh VARS copy (UEFI needs writable EFI variables)
Copy-Item $OVMF_VARS_SRC $OVMF_VARS_TMP -Force

# Clear previous logs
if (Test-Path $OO_UART)    { Remove-Item $OO_UART    -Force }
if (Test-Path $SERIAL_LOG) { Remove-Item $SERIAL_LOG -Force }

# Build QEMU args
$qemu_args = @(
    "-machine", "q35,accel=$Accel",
    "-cpu",     "max",
    "-m",       "$MemMB",
    "-drive",   "if=pflash,format=raw,readonly=on,file=$OVMF_CODE",
    "-drive",   "if=pflash,format=raw,file=$OVMF_VARS_TMP",
    "-drive",   "format=raw,file=$IMG",
    "-serial",  "file:$OO_UART",
    "-serial",  "file:$SERIAL_LOG",
    "-monitor", "none"
)

if ($Interactive) {
    $qemu_args += @("-vga", "std")
    Write-Host "  Mode: Interactive (SDL window)" -ForegroundColor Cyan
} else {
    $qemu_args += @("-display", "none")
    Write-Host "  Mode: Headless (UART → $OO_UART)" -ForegroundColor Cyan
}

# Live UART tail job
$tail_job = $null
if ($TailUart) {
    Write-Host "  Tailing UART live..." -ForegroundColor Gray
    $tail_job = Start-Job -ScriptBlock {
        param($logpath)
        $pos = 0
        while ($true) {
            Start-Sleep -Milliseconds 300
            if (Test-Path $logpath) {
                $lines = Get-Content $logpath -ErrorAction SilentlyContinue
                if ($lines -and $lines.Count -gt $pos) {
                    $lines[$pos..($lines.Count - 1)] | ForEach-Object {
                        Write-Output "  [UART] $_"
                    }
                    $pos = $lines.Count
                }
            }
        }
    } -ArgumentList $OO_UART
}

Write-Host "  Launching QEMU..." -ForegroundColor Green
Write-Host ""

# Launch with optional timeout
if ($TimeoutSec -gt 0) {
    $proc = Start-Process -FilePath $QEMU -ArgumentList $qemu_args -PassThru -NoNewWindow
    $ended = $proc.WaitForExit($TimeoutSec * 1000)
    if (-not $ended) {
        Write-Host "  [Timeout] ${TimeoutSec}s reached — stopping QEMU" -ForegroundColor Yellow
        $proc.Kill()
        $exit_code = 0  # timeout = not a crash
    } else {
        $exit_code = $proc.ExitCode
    }
} else {
    & $QEMU @qemu_args
    $exit_code = $LASTEXITCODE
}

# Stop tail
if ($tail_job) {
    $pending = Receive-Job -Job $tail_job
    $pending | ForEach-Object { Write-Host $_ -ForegroundColor DarkCyan }
    Stop-Job -Job $tail_job
    Remove-Job -Job $tail_job -Force
}

Write-Host ""
Write-Host "  QEMU exited (code: $exit_code)" -ForegroundColor $(if ($exit_code -eq 0) { "Green" } else { "Yellow" })

# ── POST-BOOT REPORT ──────────────────────────────────────────────────────────
HDR "── Post-Boot: Organism Health Report ───────────────────────"

# Parse OO UART for key events
if (Test-Path $OO_UART) {
    $uart = Get-Content $OO_UART -ErrorAction SilentlyContinue
    $event_count = ($uart | Where-Object { $_ -match '\[oo-event\]' }).Count
    $warn_count  = ($uart | Where-Object { $_ -match 'ALERT|WHITE|WARN' }).Count
    $boot_ok     = ($uart | Where-Object { $_ -match 'Soma|united_bus|OO.*init|boot' }).Count -gt 0

    Write-Host "  UART events  : $event_count" -ForegroundColor Cyan
    Write-Host "  Alerts/Warns : $warn_count" -ForegroundColor $(if ($warn_count -gt 0) { "Yellow" } else { "Green" })
    Write-Host "  Boot signals : $(if ($boot_ok) { 'detected' } else { 'none (model may not have loaded yet)' })" -ForegroundColor $(if ($boot_ok) { "Green" } else { "Gray" })

    # Show last 20 UART lines
    Write-Host ""
    Write-Host "  Last UART output:" -ForegroundColor Gray
    $uart | Select-Object -Last 20 | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkGray }
} else {
    WARN "No UART log generated (boot may have been too short)"
}

# Show serial log tail
if (Test-Path $SERIAL_LOG) {
    $serial = Get-Content $SERIAL_LOG -ErrorAction SilentlyContinue
    if ($serial -and $serial.Count -gt 0) {
        Write-Host ""
        Write-Host "  Serial log (last 10):" -ForegroundColor Gray
        $serial | Select-Object -Last 10 | ForEach-Object { Write-Host "    $_" -ForegroundColor DarkGray }
    }
}

Write-Host ""
Write-Host "══════════════════════════════════════════════════════════" -ForegroundColor Magenta
Write-Host "  OO Boot $(if ($exit_code -eq 0) { 'COMPLETE' } else { "EXIT $exit_code" })" -ForegroundColor $(if ($exit_code -eq 0) { "Green" } else { "Yellow" })
Write-Host "  UART log : $OO_UART" -ForegroundColor Gray
Write-Host "══════════════════════════════════════════════════════════" -ForegroundColor Magenta
Write-Host ""

exit $exit_code
