# oo-launch.ps1 - Operating Organism Swarm Launcher
# =====================================================================
param(
    [switch]$Interactive    = $false,
    [switch]$Rebuild        = $false,
    [switch]$RebuildImage   = $false,
    [switch]$NoHUD          = $false,
    [ValidateSet("stories15M","cortex_oo","diop_model","diop_architect","stories110M")]
    [string]$Model          = "stories15M",
    [int]$Nodes             = 1,
    [int]$TimeoutSec        = 120
)

$ErrorActionPreference = "Continue"
$ROOT    = "C:\Users\djibi\OneDrive\Bureau\baremetal"
$LLMROOT = "$ROOT\llm-baremetal"
$IMGPATH = "$LLMROOT\llm-baremetal-boot.img"
$QEMU    = "C:\Program Files\qemu\qemu-system-x86_64.exe"
$OVMF    = "$ROOT\swarm_temp\ovmf.fd"

function Title { param($s) Write-Host "`n== $s ==" -ForegroundColor Cyan }
function Ok    { param($s) Write-Host "  [OK] $s" -ForegroundColor Green }
function Info  { param($s) Write-Host "  . $s" -ForegroundColor Gray }

Set-Location $ROOT
Title "OO Swarm Launch"

# 1. Sélection modèle (Simplifié pour le test)
Info "Modele: $Model"
$MODEL_PATH = "$LLMROOT\bootable_img_temp\stories15M.bin"

# 2. Lancement de l'Essaim
Title "Lancement de l'Essaim ($Nodes noeuds)"
$qemuProcesses = @()

$SWARM_DIR = "$ROOT\swarm_temp"
if (-not (Test-Path $SWARM_DIR)) { New-Item -ItemType Directory -Path $SWARM_DIR -Force | Out-Null }

for ($i = 1; $i -le $Nodes; $i++) {
    $NODE_ID = "NODE_$i"
    $NODE_WORK_DIR = "$SWARM_DIR\$NODE_ID"
    $NODE_UART = "$ROOT\OO_UART_$NODE_ID.log"
    $NODE_VARS = "$env:TEMP\edk2-x86_64-vars-$NODE_ID.fd"
    
    Info "Booting $NODE_ID..."
    if (-not (Test-Path $NODE_WORK_DIR)) { New-Item -ItemType Directory -Path $NODE_WORK_DIR -Force | Out-Null }
    
    $NODE_IMG = "$NODE_WORK_DIR\boot.img"
    Copy-Item $IMGPATH $NODE_IMG -Force
    
    $vga = if ($Interactive) { "-vga std" } else { "-display none" }
    $argString = "-machine q35 -cpu max -m 1024 -drive if=pflash,format=raw,readonly=on,file=C:\OO\ovmf.fd -drive format=raw,file=`"$NODE_IMG`",if=ide -serial file:`"$NODE_UART`" -name OO_$NODE_ID -no-reboot $vga"
    
    Info "Cmd: qemu-system-x86_64 $argString"
    $p = Start-Process -FilePath $QEMU -ArgumentList $argString -PassThru
    $qemuProcesses += $p
    Ok "$NODE_ID (PID $($p.Id))"
    Start-Sleep -Seconds 2
}

Write-Host "`nESSAIM ACTIF. Ctrl+C pour arreter." -ForegroundColor Yellow

try {
    while ($true) {
        $running = $qemuProcesses | Where-Object { -not $_.HasExited }
        if ($running.Count -eq 0) { break }
        Start-Sleep -Seconds 1
    }
} finally {
    foreach ($p in $qemuProcesses) { if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force } }
    Title "Essaim dissout."
}
